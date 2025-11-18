#pragma once
/*
VariableTable.h
Templated table to store variable families by an enum.
- VariableTable<Enum, MAX> supports set(Enum, VariableGroup) and var(Enum, indices...).

Example:
  enum class MyVars { X, Y, MAKESPAN, MAX };
  VariableTable<MyVars, (size_t)MyVars::MAX> vars;
  vars.set(MyVars::X, VariableFactory::create(model, GRB_BINARY, ..., n1,n2));
  auto v = vars.var(MyVars::X, i, j);
*/

#include <array>
#include "VariableGroup.h"

namespace mini {

    template<typename EnumT, size_t MAX>
    class VariableTable {
        std::array<VariableGroup, MAX> table;
    public:
        void set(EnumT f, VariableGroup&& g) { table[static_cast<size_t>(f)] = std::move(g); }
        VariableGroup& get(EnumT f) { return table[static_cast<size_t>(f)]; }
        VariableGroup& operator()(EnumT f) { return get(f); }

        template<typename... Indices>
        GRBVar& var(EnumT f, Indices... idx) {
            if constexpr (sizeof...(idx) == 0) {
                return table[static_cast<size_t>(f)].scalar();
            }
            else {
                return table[static_cast<size_t>(f)].at(idx...);
            }
        }
    };

} // namespace mini
