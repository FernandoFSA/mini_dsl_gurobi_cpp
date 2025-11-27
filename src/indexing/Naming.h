#pragma once
/*
Naming.h
Unified naming system for variables and constraints.

Features:
- Compile-time name generation control
- Multi-dimensional indexing names
- Zero overhead when DEBUG_NAMES=false

Examples:
  // Variable naming
  std::string varName = naming::nameND("X", i, j, k); // "X[i,j,k]"

  // Constraint naming
  std::string constrName = naming::nameND("constr", i); // "constr[i]"

  // General string building
  std::string name = naming::make_name("prefix_", base, "_suffix");
*/

#include <string>
#include <sstream>
#include <vector>
#include <type_traits>

// Debug naming control - automatically follows _DEBUG
#ifdef _DEBUG
constexpr bool DEBUG_NAMES = true;
#else
constexpr bool DEBUG_NAMES = false;
#endif

namespace mini::naming {

    /// Concatenate multiple parts into a name
    template<typename... Args>
    inline std::string make_name(Args&&... parts) {
        if constexpr (!DEBUG_NAMES) return "";
        else {
            std::ostringstream oss;
            ((oss << std::forward<Args>(parts)), ...);
            return oss.str();
        }
    }

    /// Create name with multi-dimensional indices
    inline std::string nameND(const std::string& base, const std::vector<int>& indices) {
        if constexpr (!DEBUG_NAMES) return "";
        else {
            if (indices.empty()) return base;
            std::string result = base + "[";
            for (size_t i = 0; i < indices.size(); ++i) {
                result += std::to_string(indices[i]);
                if (i + 1 < indices.size()) result += ",";
            }
            result += "]";
            return result;
        }
    }

    /// Variadic version for direct indices
    template<typename... Indices>
    inline std::string nameND(const std::string& base, Indices... idx) {
        static_assert((std::is_integral_v<Indices> && ...), "Indices must be integers");
        if constexpr (!DEBUG_NAMES) return "";
        else {
            std::vector<int> indices = { static_cast<int>(idx)... };
            return nameND(base, indices);
        }
    }

    // Macro for convenient usage
#define MINI_MAKE_NAME(...) ::mini::naming::make_name(__VA_ARGS__)

} // namespace mini::naming