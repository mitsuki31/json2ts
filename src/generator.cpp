// SPDX-FileCopyrightText: 2026 Ryuu Mitsuki <dhefam31@proton.me>
// SPDX-License-Identifier: GPL-3.0-only

#include <sstream>

#include <json2ts/types.h>
#include <json2ts/generator.h>
#include <json2ts/utils.h>

namespace json2ts::generator {

constexpr std::string ts_type_for(const nlohmann::json& j) noexcept(true) {
    if (j.is_string())                                  return "string";
    if (j.is_number_integer() || j.is_number_float())   return "number";
    if (j.is_boolean())                                 return "boolean";
    if (j.is_null())                                    return "null";
    if (j.is_array())                                   return j.empty()
                                                            ? "any[]"
                                                            : ts_type_for(j[0]) + "[]";
    return "any";  // object handled separately
}

const size_t build_interfaces_recursive(const nlohmann::json& j,
                                        const std::string& name,
                                        std::vector<InterfaceDef>& defs,
                                        const UnorderedCommentsMap& comments_by_key) {
    std::ostringstream oss;
    // Interface-level comment
    auto iface_comment_it = comments_by_key.find(name);
    if (iface_comment_it != comments_by_key.end() && !iface_comment_it->second.empty()) {
        oss << "/**\n";
        for (const auto& line : iface_comment_it->second) {
            oss << " * " << line << "\n";
        }
        oss << " */\n";
    }
    oss << "export interface " << name << " {\n";

    for (auto it = j.begin(); it != j.end(); ++it) {
        std::string prop_name = it.key();
        std::string prop_ident = utils::sanitize_identifier(prop_name);

        // Property-level comment: prefer raw key, fallback to sanitized key
        auto comment_key_it = comments_by_key.find(prop_name);
        auto comment_ident_it = comments_by_key.find(prop_ident);
        auto comment_it = comment_key_it != comments_by_key.end() ? comment_key_it : comment_ident_it;

        if (comment_it != comments_by_key.end() && !comment_it->second.empty()) {
            oss << "  /**\n";
            for (const auto& line : comment_it->second) {
                oss << "   * " << line << "\n";
            }
            oss << "   */\n";
        }

        oss << "  " << prop_ident << ": ";
        if (it->is_object()) {
            std::string sub_name = name + utils::sanitize_identifier(
                utils::to_pascal_case(prop_name)
            );
            oss << sub_name << ";\n";
            // recurse
            build_interfaces_recursive(*it, sub_name, defs, comments_by_key);
        } else if (it->is_array()) {
            if (it->empty()) {
                oss << "any[];\n";
            } else {
                const nlohmann::json& first = (*it)[0];
                if (first.is_object()) {
                    std::string sub_name = name + utils::sanitize_identifier(
                        utils::to_pascal_case(prop_name)
                    );
                    oss << sub_name << "[];\n";
                    build_interfaces_recursive(first, sub_name, defs, comments_by_key);
                } else {
                    // Collect the type of every element, keeping first-seen order
                    // and dropping duplicates.
                    std::vector<std::string> types;
                    for (const auto& elem : *it) {
                        std::string t = ts_type_for(elem);
                        if (std::find(types.begin(), types.end(), t) == types.end()) {
                            types.push_back(std::move(t));
                        }
                    }

                    if (types.size() == 1) {
                        oss << types[0] << "[];\n";
                    } else {
                        oss << "(";
                        for (std::size_t i = 0; i < types.size(); ++i) {
                            if (i != 0) oss << " | ";
                            oss << types[i];
                        }
                        oss << ")[];\n";
                    }
                }
            }
        } else {
            oss << ts_type_for(*it) << ";\n";
        }
    }

    oss << "}\n";

    defs.push_back({name, oss.str()});
    return defs.size() - 1;
}

} // namespace json2ts::generator
