#include <cctype>

#include <json2ts/utils.h>

namespace json2ts::utils {

std::string sanitize_identifier(const std::string& s) noexcept(true) {
    std::string out;
    out.reserve(s.size() + 1);

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

std::string to_pascal_case(const std::string& s) noexcept(true) {
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

} // namespace json2ts::utils
