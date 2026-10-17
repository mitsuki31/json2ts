// SPDX-FileCopyrightText: 2026 Ryuu Mitsuki <dhefam31@proton.me>
// SPDX-License-Identifier: GPL-3.0-only

#include <fstream>
#include <filesystem>
#include <iostream>
#include <unordered_set>
#include <optional>

#include <json2ts/cli.h>
#include <json2ts/parser.h>
#include <json2ts/generator.h>
#include <json2ts/utils.h>

namespace json2ts {

int cli::parse_args(int argc, char** argv, std::string& input_file,
                    std::string& output_file, std::string& interface_name,
                    bool& no_comments, bool& use_dts, bool& use_ts, bool& has_custom_name) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        bool has_remain = i + 1 < argc;

        if (arg == "--output" && has_remain) {
            output_file = argv[++i];
        } else if (arg == "--name" && has_remain) {
            interface_name = argv[++i];
        } else if (arg == "--no-comments") {
            no_comments = true;
        } else if (arg == "--dts") {
            use_dts = true;
        } else if (arg == "--ts") {
            use_ts = true;
        } else if (arg == "--help") {
            std::cout << "json2ts - Convert JSON to TypeScript .d.ts / .ts interface file\n"
                      << "\n"
                      << "Usage:\n"
                      << "  json2ts <input.json> [--output <output>] [--name <InterfaceName>] [--dts | --ts]\n"
                      << "\n"
                      << "Options:\n"
                      << "  --output <file>  Output filename (extension enforced)\n"
                      << "  --name <Name>    Interface name (default: filename stem or single root key)\n"
                      << "  --dts            Output file with '.d.ts' extension (default)\n"
                      << "  --ts             Output file with '.ts' extension\n"
                      << "  --no-comments    Exclude JSON comments from output (still stripped for parsing)\n"
                      << "  --help           Show this help message"
                      << std::endl;
            exit(EXIT_SUCCESS);  // Force exit, do not use `return 0`
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << std::endl;
            return 1;
        } else {
            if (input_file.empty()) {
                input_file = arg;
            } else {
                std::cerr << "Unexpected argument: " << arg << std::endl;
                return 1;
            }
        }
    }

    if (input_file.empty()) {
        std::cerr << "Usage: json2ts <input.json> "
                  << "[--output <output>] [--name <InterfaceName>] [--dts | --ts]"
                  << std::endl;
        return 1;
    }

    return 0;
}

int cli::process_file(const std::string& input_file,
                 const std::string& output_file,
                 std::string interface_name,
                 bool no_comments, bool use_dts, bool use_ts,
                 std::optional<bool> is_interface_name_from_cli) {
    // Read input file
    std::ifstream infile(input_file);
    if (!infile) {
        std::cerr << "Failed to open input file: " << input_file << std::endl;
        return 1;
    }
    std::string raw((std::istreambuf_iterator<char>(infile)),
                     std::istreambuf_iterator<char>());

    // Preprocess JSON
    std::vector<ExtractedComment> comments;
    std::string cleaned = parser::preprocess_json(raw, no_comments, comments);

    // Parse JSON
    nlohmann::json data;
    try {
        data = nlohmann::json::parse(cleaned);
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    if (!data.is_object()) {
        std::cerr << "Root JSON value must be an object to generate an interface." << std::endl;
        return 1;
    }

    // Build comments map
    auto comments_by_key = parser::build_comments_by_key(comments);

    // Determine interface name if not provided
    if (interface_name.empty()) {
        if (data.size() == 1) {
            interface_name = std::string(data.begin().key());
        }
        if (interface_name.empty()) {
            std::filesystem::path p(input_file);
            interface_name = p.stem().string();
        }
    }

    // Sanitize interface name
    interface_name = utils::sanitize_identifier(interface_name.empty()
        ? utils::to_pascal_case(std::filesystem::path(input_file).stem().string())
        : is_interface_name_from_cli.value_or(false)
            ? interface_name
            : utils::to_pascal_case(interface_name)
    );
    if (interface_name.empty() || interface_name == "_") interface_name = "Root";

    // Build interfaces
    std::vector<InterfaceDef> defs;
    generator::build_interfaces_recursive(data, interface_name, defs, comments_by_key);

    // Output
    std::string output_path;
    std::string ext = use_ts ? ".ts" : ".d.ts";
    if (!output_file.empty()) {
        std::filesystem::path out_path(output_file);
        std::string filename_str = out_path.filename().string();
        bool has_ext = filename_str.find('.') != std::string::npos;
        bool ends_with_chosen = filename_str.size() >= ext.size()
            && filename_str.substr(filename_str.size() - ext.size()) == ext;

        if (!has_ext) {
            out_path = out_path.string() + ext;
        } else if (!ends_with_chosen) {
            out_path.replace_extension(ext);
        }
        output_path = out_path.string();
    } else {
        std::filesystem::path p(input_file);
        std::string stem = p.stem().string();
        p = p.parent_path() / (stem + ext);
        output_path = p.string();
    }

    std::ofstream out_file(output_path);
    if (!out_file) {
        std::cerr << "Failed to open output file: " << output_path << std::endl;
        return 1;
    }

    // Write file header comment (optional)
    if (!no_comments) {
        std::unordered_set<std::string> seen;
        std::vector<std::string> file_comments;

        for (const auto& c : comments) {
            if (seen.insert(c.text).second) {
                file_comments.push_back(c.text);
            }
        }

        if (!file_comments.empty()) {
            out_file << "/**\n";
            for (const auto& line : file_comments) {
                out_file << " * " << line << "\n";
            }
            out_file << " */\n";
        }
    }

    // Write each interface
    std::unordered_set<std::string> emitted;
    for (const auto& def : defs) {
        if (emitted.insert(def.name).second) {
            out_file << def.content;
        }
    }

    out_file << std::endl;
    std::cout << "Generated: " << output_path << std::endl;
    return 0;
}

int cli::run(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file;
    std::string interface_name;
    bool no_comments = false;
    bool use_dts = false;
    bool use_ts = false;
    bool custom_name = false;

    int parse_res = parse_args(argc, argv, input_file, output_file, interface_name,
                               no_comments, use_dts, use_ts, custom_name);
    if (parse_res != 0) return parse_res;

    return process_file(input_file, output_file,
                        interface_name, no_comments,
                        use_dts, use_ts, custom_name);
}

} // namespace json2ts
