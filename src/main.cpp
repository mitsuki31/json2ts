#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <algorithm>
#include <filesystem>
#include <regex>
#include <set>
#include <cctype>

using json = nlohmann::json;
namespace fs = std::filesystem;

// Extracted comments with approximate key association
struct ExtractedComment {
    std::string text;
    std::string key;
};

std::string sanitize_identifier(const std::string& s) noexcept {
    std::string out;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];

        if (std::isalpha(static_cast<unsigned char>(c))
            || c == '_'
            || (i > 0 && std::isdigit(static_cast<unsigned char>(c)))
        ) {
            out += c;
        } else {
            out += '_';
        }
    }

    if (out.empty()) out = "_";
    if (std::isdigit(static_cast<unsigned char>(out[0]))) out = "_" + out;

    return out;
}

std::string strip_whitespace_around(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
        ++start;

    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
        --end;

    return s.substr(start, end - start);
}

std::string preprocess_json(const std::string& raw,
                            std::vector<ExtractedComment>& comments) {
    std::string cleaned;
    size_t i = 0;

    while (i < raw.size()) {
        if (i + 1 < raw.size() && raw[i] == '/' && raw[i + 1] == '/') {
            std::string comment_text;
            i += 2;

            while (i < raw.size() && raw[i] != '\n') {
                comment_text += raw[i];
                ++i;
            }

            std::string trimmed = strip_whitespace_around(comment_text);
            comments.push_back({trimmed, ""});

            // Try to associate with next quoted key
            std::string rest = raw.substr(i);
            std::regex key_regex("\"([^\"]+)\"");
            std::smatch match;
            if (std::regex_search(rest, match, key_regex)) {
                comments.back().key = match[1];
            }
        } else if (i + 1 < raw.size() && raw[i] == '/' && raw[i + 1] == '*') {
            std::string comment_text;
            i += 2;

            while (i + 1 < raw.size()) {
                if (raw[i] == '*' && raw[i + 1] == '/') {
                    i += 2;
                    break;
                }

                comment_text += raw[i];
                ++i;
            }

            std::string trimmed = strip_whitespace_around(comment_text);
            comments.push_back({trimmed, ""});

            std::string rest = raw.substr(i);
            std::regex key_regex("\"([^\"]+)\"");
            std::smatch match;
            if (std::regex_search(rest, match, key_regex)) {
                comments.back().key = match[1];
            }
        } else {
            cleaned += raw[i];
            ++i;
        }
    }
    return cleaned;
}

const std::string ts_type_for(const json& j) noexcept {
    if (j.is_string()) return "string";
    if (j.is_number_integer() || j.is_number_float()) return "number";
    if (j.is_boolean()) return "boolean";
    if (j.is_null()) return "null";
    if (j.is_array()) {
        if (j.empty()) return "any[]";
        return ts_type_for(j[0]) + "[]";
    }
    // object handled separately
    return "any";
}

inline bool is_primitive(const json& j) noexcept {
    return (j.is_string()
        || j.is_number_integer()
        || j.is_number_float()
        || j.is_boolean()
        || j.is_null()
        || j.is_array());
}

const std::string to_pascal_case(const std::string& s) {
    std::string out;
    bool capitalize = true;

    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (std::isalnum(static_cast<unsigned char>(c))) {
            if (capitalize) {
                out += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                capitalize = false;
            } else {
                out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
        } else {
            capitalize = true;
        }
    }
    return sanitize_identifier(out);
}

void generate_interface(const std::string& interface_name,
                        const json& obj,
                        std::ostringstream& out,
                        const std::map<std::string,
                        std::vector<std::string>>& comments_map) {
    std::vector<std::string> doc_lines;
    auto it = comments_map.find(interface_name);
    if (it != comments_map.end() && !it->second.empty()) {
        for (const auto& c : it->second) {
            doc_lines.push_back(c);
        }
    }
    if (!doc_lines.empty()) {
        out << "/**\n";
        for (const auto& line : doc_lines) {
            out << " * " << line << "\n";
        }
        out << " */\n";
    }

    out << "export interface " << interface_name << " {\n";
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        std::string prop_name = it.key();
        std::string prop_ident = sanitize_identifier(prop_name);
        std::vector<std::string> prop_comments;

        auto c_it = comments_map.find(prop_ident);
        if (c_it != comments_map.end()) {
            prop_comments = c_it->second;
        }

        // Also try key-based lookup
        auto c_key_it = comments_map.find(prop_name);
        if (c_key_it != comments_map.end()) {
            prop_comments = c_key_it->second;
        }

        if (!prop_comments.empty()) {
            out << "  /**\n";
            for (const auto& line : prop_comments) {
                out << "   * " << line << "\n";
            }
            out << "   */\n";
        }

        out << "  " << prop_ident << ": ";
        if (it->is_object()) {
            std::string sub_name = interface_name + sanitize_identifier(
                to_pascal_case(prop_name)
            );
            out << sub_name << ";\n";
        } else if (it->is_array()) {
            if (it->empty()) {
                out << "any[];\n";
            } else {
                const json& first = (*it)[0];
                if (first.is_object()) {
                    std::string sub_name = interface_name + sanitize_identifier(
                        to_pascal_case(prop_name)
                    );
                    out << sub_name << "[];\n";
                } else {
                    out << ts_type_for(*it) << ";\n";
                }
            }
        } else {
            out << ts_type_for(*it) << ";\n";
        }
    }
    out << "}\n";
}

struct InterfaceDef {
    std::string name;
    std::string content;
};

void build_interfaces_recursive(const json& j,
                                const std::string& name,
                                std::vector<InterfaceDef>& defs,
                                std::map<std::string,
                                std::vector<std::string>>& comments_by_key) {
    std::vector<InterfaceDef> nested_all;
    std::ostringstream oss;

    oss << "export interface " << name << " {\n";
    for (auto it = j.begin(); it != j.end(); ++it) {
        std::string prop_name = it.key();
        std::string prop_ident = sanitize_identifier(prop_name);
        std::vector<std::string> prop_comments;

        auto c_key_it = comments_by_key.find(prop_name);
        if (c_key_it != comments_by_key.end()) prop_comments = c_key_it->second;

        auto c_ident_it = comments_by_key.find(prop_ident);
        if (c_ident_it != comments_by_key.end()) prop_comments = c_ident_it->second;

        if (!prop_comments.empty()) {
            oss << "  /**\n";
            for (const auto& line : prop_comments) {
                oss << "   * " << line << "\n";
            }
            oss << "   */\n";
        }

        oss << "  " << prop_ident << ": ";
        if (it->is_object()) {
            std::string sub_name = name + sanitize_identifier(
                to_pascal_case(prop_name)
            );
            oss << sub_name << ";\n";
            
            std::vector<InterfaceDef> temp_nested_obj;
            build_interfaces_recursive(*it,
                                        sub_name,
                                        temp_nested_obj,
                                        comments_by_key);
            nested_all.insert(
                nested_all.end(),
                temp_nested_obj.begin(),
                temp_nested_obj.end()
            );
        } else if (it->is_array()) {
            if (it->empty()) {
                oss << "any[];\n";
            } else {
                const json& first = (*it)[0];
                if (first.is_object()) {
                    std::string sub_name = name + sanitize_identifier(
                        to_pascal_case(prop_name)
                    );
                    oss << sub_name << "[];\n";
                    
                    std::vector<InterfaceDef> temp_nested_arr;
                    build_interfaces_recursive(first,
                                               sub_name,
                                               temp_nested_arr,
                                               comments_by_key);
                    nested_all.insert(
                        nested_all.end(),
                        temp_nested_arr.begin(),
                        temp_nested_arr.end()
                    );
                } else {
                    oss << ts_type_for(*it) << ";\n";
                }
            }
        } else {
            oss << ts_type_for(*it) << ";\n";
        }
    }
    oss << "}\n";
    defs.push_back({name, oss.str()});
    defs.insert(defs.end(), nested_all.begin(), nested_all.end());
}



// =================== CLI APP DRIVER ================ //

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file;
    std::string interface_name;
    bool interface_name_from_cli = false;
    bool use_dts = false;
    bool use_ts = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        bool has_remain = i + 1 < argc;

        if (arg == "--output" && has_remain) {
            output_file = argv[++i];
        } else if (arg == "--name" && has_remain) {
            interface_name = argv[++i];
            interface_name_from_cli = true;
        } else if (arg == "--dts") {
            use_dts = true;
        } else if (arg == "--ts") {
            use_ts = true;
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

    if (!use_dts && !use_ts) {
        use_ts = true;  // default to use '.ts' extension
    } else if (use_dts && use_ts) {
        std::cerr << "Error: Cannot specify both --dts and --ts" << std::endl;
        return 1;
    }

    if (input_file.empty()) {
        std::cerr
            << "Usage: json2ts <input.json> "
            << "[--output <output>] [--name <InterfaceName>] [--dts | --ts]"
            << std::endl;
        return 1;
    }

    std::ifstream infile(input_file);
    if (!infile) {
        std::cerr << "Failed to open input file: " << input_file << std::endl;
        return 1;
    }
    std::string raw((std::istreambuf_iterator<char>(infile)),
                    std::istreambuf_iterator<char>());

    std::vector<ExtractedComment> comments;
    std::string cleaned = preprocess_json(raw, comments);

    // Build comments map (best effort association)
    std::map<std::string, std::vector<std::string>> comments_by_key;
    for (const auto& c : comments) {
        std::string key_for_lookup = c.key.empty()
            ? sanitize_identifier(c.text)
            : sanitize_identifier(c.key);
        // Try to use the associated key directly if found; otherwise use sanitized text
        std::string target_key = c.key.empty()
            ? sanitize_identifier(c.text)
            : c.key;
        comments_by_key[target_key].push_back(c.text);
    }
    // Also map comments by their text as a fallback
    for (const auto& c : comments) {
        std::string text_key = sanitize_identifier(c.text);

        if (comments_by_key.find(text_key) == comments_by_key.end()) {
            comments_by_key[text_key].push_back(c.text);
        }
    }

    json data;
    try {
        data = json::parse(cleaned);
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    if (!data.is_object()) {
        std::cerr
            << "Root JSON value must be an object to generate an interface."
            << std::endl;
        return 1;
    }

    if (interface_name.empty()) {
        // Check if JSON root is object with a single key (use that key name)
        if (data.size() == 1) {
            interface_name = data.begin().key();
            // But the interface should describe the root object, not just the single value.
            // Let's use the key name if appropriate, else filename.
        }
        if (interface_name.empty()) {
            fs::path p(input_file);
            interface_name = p.stem().string();
        }
    }

    interface_name = sanitize_identifier(
        interface_name_from_cli ? interface_name : to_pascal_case(interface_name)
    );
    if (interface_name.empty() || interface_name == "_") interface_name = "Root";

    std::vector<InterfaceDef> defs;
    build_interfaces_recursive(data, interface_name, defs, comments_by_key);

    std::ostringstream out;
    // File-level JSDoc from all comments (best effort)
    std::vector<std::string> file_comments;
    for (const auto& c : comments) {
        if (std::find(file_comments.begin(),
                      file_comments.end(),
                      c.text) == file_comments.end()) {
            file_comments.push_back(c.text);
        }
    }
    if (!file_comments.empty()) {
        out << "/**\n";
        for (const auto& line : file_comments) {
            out << " * " << line << "\n";
        }
        out << " */\n";
    }

    std::set<std::string> emitted;
    for (const auto& def : defs) {
        if (emitted.insert(def.name).second) {
            out << def.content;
        }
    }

    std::string output_path;
    std::string ext = use_ts ? ".ts" : ".d.ts";
    if (!output_file.empty()) {
        fs::path out_path(output_file);
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
        fs::path p(input_file);
        std::string stem = p.stem().string();
        p = p.parent_path() / (stem + ext);
        output_path = p.string();
    }

    std::ofstream out_file(output_path);
    if (!out_file) {
        std::cerr << "Failed to open output file: " << output_path << std::endl;
        return 1;
    }
    out_file << out.str();
    std::cout << "Generated: " << output_path << std::endl;

    return 0;
}
