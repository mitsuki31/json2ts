// SPDX-FileCopyrightText: 2026 Ryuu Mitsuki <dhefam31@proton.me>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef JSON2TS_PARSER_H
#define JSON2TS_PARSER_H

#include <string>
#include <vector>

#include <json2ts/types.h>

namespace json2ts::parser {

std::string preprocess_json(const std::string& raw,
                            bool no_comments,
                            std::vector<ExtractedComment>& comments);

UnorderedCommentsMap build_comments_by_key(const std::vector<ExtractedComment>& comments);

} // namespace json2ts::parser

#endif // JSON2TS_PARSER_H
