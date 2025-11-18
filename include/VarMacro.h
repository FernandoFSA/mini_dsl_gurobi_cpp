#pragma once
#include <array>
#include <string>
#include <string_view>
#include <utility>
#include <sstream>

namespace mini::dsl {
inline std::string concat_names(std::string_view a, std::string_view b) {
    std::string s;
    s.reserve(a.size() + b.size());
    s.append(a);
    s.append(b);
    return s;
}
template<typename... Args>
inline std::string make_name(Args&&... parts) {
    std::ostringstream oss;
    ((oss << std::forward<Args>(parts)), ...);
    return oss.str();
}
#define MINI_DSL_MAKE_NAME(...) ::mini::dsl::make_name(__VA_ARGS__)
}
