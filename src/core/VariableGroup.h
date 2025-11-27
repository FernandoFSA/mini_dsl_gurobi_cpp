#pragma once
/*
VariableGroup.h
Recursive N-D container for GRBVar with zero-copy access.

Features:
- Scalar (0-D) and N-D nested variables
- Zero-overhead element access via at(i,j,k)
- Memory-efficient tree structure

Examples:
  // Create 3D variable group
  VariableGroup X = VariableFactory::add(model, GRB_BINARY, 0, 1, "X", 10, 20, 30);

  // Access elements
  model.addConstr(X.at(i, j, k) == 1);
  model.addConstr(X(i, j, k) <= 0.5);  // Operator() syntax
*/

#include <vector>
#include <stdexcept>
#include <type_traits>
#include "gurobi_c++.h"

namespace mini {

    class VariableGroup {
    public:
        struct Node {
            GRBVar scalar;
            std::vector<Node> children;

            Node() = default;
            Node(const GRBVar& v) : scalar(v) {}
            Node(size_t n) : children(n) {}
        };

    private:
        Node root;
        int dims = 0;

    public:
        VariableGroup() = default;
        VariableGroup(Node&& r, int d) : root(std::move(r)), dims(d) {}
        explicit VariableGroup(const GRBVar& v) : root(Node(v)), dims(0) {}

        int dimension() const { return dims; }

        /// Access element with bounds checking
        template<typename... Indices>
        GRBVar& at(Indices... idx) {
            static_assert((std::is_integral_v<Indices> && ...), "Indices must be integral.");
            if (static_cast<int>(sizeof...(idx)) != dims) {
                throw std::runtime_error("VariableGroup::at(): wrong number of indices");
            }
            return atRec(root, idx...);
        }

        /// Access scalar variable
        GRBVar& scalar() {
            if (dims != 0) throw std::runtime_error("VariableGroup::scalar() called on non-scalar");
            return root.scalar;
        }

        /// Operator() syntax for cleaner code
        template<typename... I> GRBVar& operator()(I... idx) { return at(idx...); }

    private:
        // Base case: return scalar variable
        GRBVar& atRec(Node& n) { return n.scalar; }

        // Recursive case: traverse children
        template<typename First, typename... Rest>
        GRBVar& atRec(Node& n, First i, Rest... rest) {
            if (i < 0 || static_cast<size_t>(i) >= n.children.size()) {
                throw std::out_of_range("VariableGroup index out of range");
            }
            return atRec(n.children[static_cast<size_t>(i)], rest...);
        }

        friend class VariableFactory;
    };

} // namespace mini