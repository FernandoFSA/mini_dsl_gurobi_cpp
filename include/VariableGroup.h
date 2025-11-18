#pragma once
/*
VariableGroup.h
Recursive N-D container for GRBVar.
Supports:
 - scalar (0-D)
 - N-D nested children (each node is either a leaf with GRBVar or a non-leaf with children)
APIs:
 - VariableGroup(GRBVar v) -> scalar
 - VariableGroup(Node&& node, int dims) -> general N-D wrapper (used by VariableFactory)
 - at(i1,i2,...,iN) -> GRBVar&

Example:
  VariableGroup X = VariableFactory::create(model, GRB_BINARY, 0,1,"X", n1,n2,n3); // 3D
  model.addConstr( X.at(i,j,k) == 1 );
*/

#include <vector>
#include <stdexcept>
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

        template<typename... Indices>
        GRBVar& at(Indices... idx) {
            static_assert((std::is_integral_v<Indices> && ...), "Indices must be integral.");
            if ((int)sizeof...(idx) != dims) throw std::runtime_error("Wrong number of indices for VariableGroup::at()");
            return atRec(root, idx...);
        }

        GRBVar& scalar() {
            if (dims != 0) throw std::runtime_error("scalar() called on non-scalar VariableGroup");
            return root.scalar;
        }

        template<typename... I> GRBVar& operator()(I... idx) { return at(idx...); }

    private:
        GRBVar& atRec(Node& n) { return n.scalar; }

        template<typename First, typename... Rest>
        GRBVar& atRec(Node& n, First i, Rest... rest) {
            if (i < 0 || static_cast<size_t>(i) >= n.children.size()) throw std::out_of_range("VariableGroup index out of range");
            return atRec(n.children[static_cast<size_t>(i)], rest...);
        }

        friend class VariableFactory;
    };

} // namespace mini
