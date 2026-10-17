// SPDX-FileCopyrightText: 2026 Ryuu Mitsuki <dhefam31@proton.me>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef JSON2TS_UTILS_H
#define JSON2TS_UTILS_H

#include <string>

namespace json2ts::utils {

std::string sanitize_identifier(const std::string& s) noexcept(true);
std::string strip_whitespace_around(const std::string& s);
std::string to_pascal_case(const std::string& s) noexcept(true);

} // namespace json2ts::utils

#endif // JSON2TS_UTILS_H
