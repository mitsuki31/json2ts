// SPDX-FileCopyrightText: 2026 Ryuu Mitsuki <dhefam31@proton.me>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef JSON2TS_TYPES_H
#define JSON2TS_TYPES_H

#include <string>        // std::string
#include <unordered_map> // std::unordered_map
#include <vector>        // std::vector

namespace json2ts {

/**
 * @brief A documentation comment extracted from a source file.
 *
 * Pairs the raw comment text with the identifier (key) it documents.
 */
struct ExtractedComment {
    /// The full comment body, with delimiters (`//`, `/* */`) stripped.
    std::string text;

    /// The identifier this comment is associated with (e.g. a field name).
    std::string key;
};

/**
 * @brief A generated TypeScript interface.
 *
 * Holds the interface name and its fully-rendered body, ready to be
 * written to an output file without further formatting.
 */
struct InterfaceDef {
    /// The interface identifier, e.g. "User" or "ApiResponse".
    std::string name;

    /// The complete rendered interface, including `interface Name { ... }`
    /// and any leading doc comments.
    std::string content;
};

/**
 * @brief Transparent alias for `std::unordered_map`.
 *
 * @tparam K Key type. Must satisfy `std::hash<K>` unless a custom hash
 *           is used at the call site.
 * @tparam V Mapped value type.
 *
 * @note This is an alias, not a distinct type — it cannot be partially
 *       specialized, and it is interchangeable with `std::unordered_map<K,V>`.
 */
template<typename K, typename V>
using UnorderedMap = std::unordered_map<K, V>;

/**
 * @brief Lookup table mapping a key to all comments collected for it.
 *
 * Keys match `ExtractedComment::key`. The value is the list of comment
 * texts for that key, in encounter order. Key order is unspecified.
 */
using UnorderedCommentsMap = UnorderedMap<std::string, std::vector<std::string>>;

} // namespace json2ts

#endif // JSON2TS_TYPES_H
