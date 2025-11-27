#pragma once
/*
VariableFactory.h
Create variables attached to models or as independent handles.

Features:
- Single API for scalars and N-D variables
- Automatic naming with DEBUG_NAMES control
- Memory-efficient tree construction

Examples:
  // Scalar variable
  GRBVar x = VariableFactory::add(model, GRB_BINARY, 0, 1, "x");

  // 3D variable group
  VariableGroup X = VariableFactory::add(model, GRB_CONTINUOUS, 0, 100, "X", 5, 10, 15);

  // Independent variables (not attached to model)
  VariableGroup Y = VariableFactory::create(GRB_BINARY, 0, 1, "Y", 8, 8);
*/

#include <string>
#include <type_traits>
#include "gurobi_c++.h"
#include "VariableGroup.h"
#include "..\indexing\Naming.h"

namespace mini {

    class VariableFactory {
    public:
        using Node = VariableGroup::Node;

        /// Add variables to a model
        template<typename... Sizes>
        static auto add(GRBModel& model, int vtype, double lb, double ub,
            const std::string& baseName, Sizes... sizes) {
            if constexpr (sizeof...(sizes) == 0) {
                std::string name = naming::make_name(baseName);
                return model.addVar(lb, ub, 0.0, vtype, name);
            }
            else {
                Node root = addNode(model, vtype, lb, ub, baseName, sizes...);
                return VariableGroup(std::move(root), static_cast<int>(sizeof...(sizes)));
            }
        }

        /// Create independent variable handles
        template<typename... Sizes>
        static auto create(Sizes... sizes) {
            if constexpr (sizeof...(sizes) == 0) {
                return GRBVar();
            }
            else {
                Node root = createNode(sizes...);
                return VariableGroup(std::move(root), static_cast<int>(sizeof...(sizes)));
            }
        }

    private:
        // Implementation details...
        template<typename SizeType>
        static Node addNode(GRBModel& model, int vtype, double lb, double ub,
            const std::string& name, SizeType n) {
            static_assert(std::is_integral_v<SizeType>);
            Node node(static_cast<size_t>(n));
            for (SizeType i = 0; i < n; ++i) {
                std::string varName = naming::nameND(name, i);
                node.children[static_cast<size_t>(i)] = Node(model.addVar(lb, ub, 0.0, vtype, varName));
            }
            return node;
        }

        template<typename SizeType, typename... Sizes>
        static Node addNode(GRBModel& model, int vtype, double lb, double ub,
            const std::string& name, SizeType n, Sizes... sizes) {
            static_assert(std::is_integral_v<SizeType>);
            Node node(static_cast<size_t>(n));
            for (SizeType i = 0; i < n; ++i) {
                std::string subName = naming::nameND(name, i);
                node.children[static_cast<size_t>(i)] = addNode(model, vtype, lb, ub, subName, sizes...);
            }
            return node;
        }

        template<typename SizeType>
        static Node createNode(SizeType n) {
            static_assert(std::is_integral_v<SizeType>);
            Node node(static_cast<size_t>(n));
            for (SizeType i = 0; i < n; ++i) {
                node.children[static_cast<size_t>(i)] = Node(GRBVar());
            }
            return node;
        }

        template<typename SizeType, typename... Sizes>
        static Node createNode(SizeType n, Sizes... sizes) {
            static_assert(std::is_integral_v<SizeType>);
            Node node(static_cast<size_t>(n));
            for (SizeType i = 0; i < n; ++i) {
                node.children[static_cast<size_t>(i)] = createNode(sizes...);
            }
            return node;
        }
    };

} // namespace mini