// SPDX-FileCopyrightText: 2026 Ryuu Mitsuki <dhefam31@proton.me>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef JSON2TS_CLI_H
#define JSON2TS_CLI_H

#include <optional>
#include <string>

namespace json2ts {

class cli {
public:
    // run parses argv, performs the conversion, returns exit code
    static int run(int argc, char* argv[]);

private:
    // class-scoped helper methods
    static int parse_args(int argc, char* argv[], std::string& input_file,
                          std::string& output_file, std::string& interface_name,
                          bool& no_comments, bool& use_dts, bool& use_ts, bool& has_custom_name);

    static int process_file(const std::string& input_file,
                            const std::string& output_file,
                            std::string interface_name,
                            bool no_comments, bool use_dts, bool use_ts,
                            std::optional<bool> is_interface_name_from_cli);
};

} // namespace json2ts

#endif // JSON2TS_CLI_H
