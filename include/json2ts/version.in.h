// SPDX-FileCopyrightText: 2026 Ryuu Mitsuki <dhefam31@proton.me>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef JSON2TS_VERSION_H
#define JSON2TS_VERSION_H

#include <string_view>

namespace json2ts::version {

// Project name
inline constexpr std::string_view kProjectName = "@PROJECT_NAME@";
// Project version
inline constexpr std::string_view kVersion     = "@PROJECT_VERSION@";

}  // namespace json2ts::version

#endif // JSON2TS_VERSION_H
