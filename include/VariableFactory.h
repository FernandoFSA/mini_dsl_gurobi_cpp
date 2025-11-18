#pragma once
/*
VariableFactory.h
Create attached and independent variables in N dimensions.

APIs:
 - create(model, vtype, lb, ub, "Name") -> GRBVar (scalar)
 - create(model, vtype, lb, ub, "Name", n1, n2, ...) -> VariableGroup (N-D)
 - createIndependent(n1, n2, ...) -> VariableGroup of unattached GRBVar handles

Notes:
 - Uses DEBUG_VARIABLE_NAMES from build (see README). If enabled, variables are named like "X[0][1][2]".
*/

#include <string>
#include <format>
#include <type_traits>
#include "gurobi_c++.h"
#include "VariableGroup.h"

namespace mini {

    class VariableFactory {
    public:
        using Node = VariableGroup::Node;

        template<typename... Sizes>
        static auto create(GRBModel& model, int vtype, double lb, double ub, const std::string& baseName, Sizes... sizes) {
            if constexpr (sizeof...(sizes) == 0) {
                std::string n = DEBUG_VARIABLE_NAMES ? baseName : "";
                return model.addVar(lb, ub, 0.0, vtype, n);
            }
            else {
                Node root = createNode(model, vtype, lb, ub, baseName, sizes...);
                return VariableGroup(std::move(root), (int)sizeof...(sizes));
            }
        }

        template<typename... Sizes>
        static auto createIndependent(Sizes... sizes) {
            if constexpr (sizeof...(sizes) == 0) {
                return GRBVar();
            }
            else {
                Node root = createIndependentNode(sizes...);
                return VariableGroup(std::move(root), (int)sizeof...(sizes));
            }
        }

    private:
        // base 1D
        template<typename SizeType>
        static Node createNode(GRBModel& model, int vtype, double lb, double ub, const std::string& name, SizeType n) {
            static_assert(std::is_integral_v<SizeType>);
            Node node((size_t)n);
            for (SizeType i = 0; i < n; ++i) {
                std::string nm = DEBUG_VARIABLE_NAMES ? std::format("{}[{}]", name, i) : "";
                node.children[(size_t)i] = Node(model.addVar(lb, ub, 0.0, vtype, nm));
            }
            return node;
        }

        // recursive N>1
        template<typename SizeType, typename... Sizes>
        static Node createNode(GRBModel& model, int vtype, double lb, double ub, const std::string& name, SizeType n, Sizes... sizes) {
            static_assert(std::is_integral_v<SizeType>);
            Node node((size_t)n);
            for (SizeType i = 0; i < n; ++i) {
                std::string sub = DEBUG_VARIABLE_NAMES ? std::format("{}[{}]", name, i) : name;
                node.children[(size_t)i] = createNode(model, vtype, lb, ub, sub, sizes...);
            }
            return node;
        }

        // independent base
        template<typename SizeType>
        static Node createIndependentNode(SizeType n) {
            static_assert(std::is_integral_v<SizeType>);
            Node node((size_t)n);
            for (SizeType i = 0; i < n; ++i) node.children[(size_t)i] = Node(GRBVar());
            return node;
        }

        template<typename SizeType, typename... Sizes>
        static Node createIndependentNode(SizeType n, Sizes... sizes) {
            static_assert(std::is_integral_v<SizeType>);
            Node node((size_t)n);
            for (SizeType i = 0; i < n; ++i) node.children[(size_t)i] = createIndependentNode(sizes...);
            return node;
        }
    };

} // namespace mini
