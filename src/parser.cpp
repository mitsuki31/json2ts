#include <fstream>
#include <sstream>
#include <cctype>

#include <json2ts/parser.h>
#include <json2ts/utils.h>

namespace json2ts::parser {

std::string preprocess_json(const std::string& raw,
                            bool no_comments,
                            std::vector<ExtractedComment>& comments) {
    std::string cleaned;
    cleaned.reserve(raw.size());
    size_t i = 0;
    std::vector<size_t> pending_comments;

    while (i < raw.size()) {
        if (raw[i] == '"') {
            const size_t quote_start = i++;
            while (i < raw.size()) {
                if (raw[i] == '\\' && i + 1 < raw.size()) {
                    i += 2;
                } else if (raw[i] == '"') {
                    ++i;
                    break;
                } else {
                    ++i;
                }
            }
            if (!pending_comments.empty()) {
                const std::string key = raw.substr(quote_start + 1,
                    (i > quote_start + 1 && raw[i - 1] == '"' ? i - quote_start - 2 : i - quote_start - 1));
                for (const size_t index : pending_comments) {
                    comments[index].key = key;
                }
                pending_comments.clear();
            }
            cleaned.append(raw, quote_start, i - quote_start);
        } else if (i + 1 < raw.size() && raw[i] == '/' && raw[i + 1] == '/') {
            std::string comment_text;
            i += 2;
            while (i < raw.size() && raw[i] != '\n') {
                comment_text += raw[i];
                ++i;
            }
            std::string trimmed = utils::strip_whitespace_around(comment_text);
            if (!no_comments) {
                comments.push_back({trimmed, ""});
                pending_comments.push_back(comments.size() - 1);
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
            std::string trimmed = utils::strip_whitespace_around(comment_text);
            if (!no_comments) {
                comments.push_back({trimmed, ""});
                pending_comments.push_back(comments.size() - 1);
            }
        } else {
            cleaned += raw[i];
            ++i;
        }
    }
    return cleaned;
}

UnorderedCommentsMap build_comments_by_key(const std::vector<ExtractedComment>& comments) {
    UnorderedCommentsMap map;

    for (const auto& c : comments) {
        std::string target_key = c.key.empty() ? utils::sanitize_identifier(c.text) : c.key;
        map[target_key].push_back(c.text);
    }

    for (const auto& c : comments) {
        std::string text_key = utils::sanitize_identifier(c.text);
        if (map.find(text_key) == map.end()) {
            map[text_key].push_back(c.text);
        }
    }
    return map;
}

} // namespace json2ts::parser
