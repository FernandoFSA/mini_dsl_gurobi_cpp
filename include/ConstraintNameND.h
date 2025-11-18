#pragma once
/*
ConstraintNameND.h
cnameND(base, {i,j,k}) -> "base[i,j,k]"
cnameND(base, i, j, k)    -> same via variadic template

DEBUG_CONSTRAINT_NAMES guard allows disabling names for performance.
*/

#include <string>
#include <vector>
#include <format>

#ifndef DEBUG_CONSTRAINT_NAMES
#define DEBUG_CONSTRAINT_NAMES 1
#endif

namespace mini::constraint {

    inline std::string cnameND(const std::string& base, const std::vector<int>& idx) {
        if (!DEBUG_CONSTRAINT_NAMES) return "";
        if (idx.empty()) return base;
        std::string s = base + "[";
        for (size_t t = 0; t < idx.size(); ++t) {
            s += std::to_string(idx[t]);
            if (t + 1 < idx.size()) s += ",";
        }
        s += "]";
        return s;
    }

    template<typename... Ints>
    inline std::string cnameND(const std::string& base, Ints... vals) {
        static_assert((std::is_integral_v<Ints> && ...), "Indices must be integers");
        std::vector<int> idx = { static_cast<int>(vals)... };
        return cnameND(base, idx);
    }

} // namespace mini::constraint