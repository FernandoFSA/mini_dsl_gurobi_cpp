#pragma once
/*
VariableTable.h
Type-safe storage for variable families using enum keys.

Features:
- Compile-time sized table
- Type-safe enum access
- Clean syntax for variable retrieval

Examples:
  enum class Vars { X, Y, Z, COUNT };
  VariableTable<Vars, (size_t)Vars::COUNT> vars;

  // Store variables
  vars.set(Vars::X, VariableFactory::add(model, GRB_BINARY, 0, 1, "X", 10, 10));

  // Access variables
  GRBVar& x_ij = vars.var(Vars::X, i, j);
  GRBVar& y = vars.var(Vars::Y);  // scalar
*/

#include <array>
#include "VariableGroup.h"

namespace mini {

    template<typename EnumT, size_t MAX>
    class VariableTable {
        std::array<VariableGroup, MAX> table;

    public:
        /// Store variable group
        void set(EnumT key, VariableGroup&& group) {
            table[static_cast<size_t>(key)] = std::move(group);
        }

        /// Store scalar GRBVar (const lvalue)
        void set(EnumT key, const GRBVar& var) {
            table[static_cast<size_t>(key)] = VariableGroup(var);
        }

        /// Store scalar GRBVar (rvalue)
        void set(EnumT key, GRBVar&& var) {
            table[static_cast<size_t>(key)] = VariableGroup(var);
        }

        /// Get variable group reference
        VariableGroup& get(EnumT key) { return table[static_cast<size_t>(key)]; }

        /// Operator() syntax for group access
        VariableGroup& operator()(EnumT key) { return get(key); }

        /// Access variable with indices (scalar if no indices)
        template<typename... Indices>
        GRBVar& var(EnumT key, Indices... idx) {
            if constexpr (sizeof...(idx) == 0) {
                return table[static_cast<size_t>(key)].scalar();
            }
            else {
                return table[static_cast<size_t>(key)].at(idx...);
            }
        }
    };

} // namespace mini