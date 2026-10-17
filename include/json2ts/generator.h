#ifndef JSON2TS_GENERATOR_H
#define JSON2TS_GENERATOR_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include <json2ts/types.h>

namespace json2ts::generator {

constexpr std::string ts_type_for(const nlohmann::json& j) noexcept(true);

const size_t build_interfaces_recursive(const nlohmann::json& j,
                                        const std::string& name,
                                        std::vector<InterfaceDef>& defs,
                                        const UnorderedCommentsMap& comments_by_key);

} // namespace json2ts::generator

#endif // JSON2TS_GENERATOR_H
