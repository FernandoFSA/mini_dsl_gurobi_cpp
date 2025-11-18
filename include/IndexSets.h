#pragma once
/*
IndexSets.h
Utilities for creating index sets and computing cartesian products.
- indexSet(n): returns vector<int> {0,...,n-1}
- cartesianProduct(sets): returns vector<vector<int>> of tuples (each tuple is one combination)

Examples:
  auto I = indexSet(5);         // {0,1,2,3,4}
  auto sets = std::vector{I, J, K};
  auto tuples = cartesianProduct(sets);
*/

#include <vector>
#include <numeric>

namespace mini::dsl {

    inline std::vector<int> indexSet(int n) {
        std::vector<int> r(n);
        std::iota(r.begin(), r.end(), 0);
        return r;
    }

    inline void cartProdRec(const std::vector<std::vector<int>>& sets,
        size_t depth,
        std::vector<int>& cur,
        std::vector<std::vector<int>>& out)
    {
        if (depth == sets.size()) {
            out.push_back(cur);
            return;
        }
        for (int v : sets[depth]) {
            cur[depth] = v;
            cartProdRec(sets, depth + 1, cur, out);
        }
    }

    inline std::vector<std::vector<int>> cartesianProduct(const std::vector<std::vector<int>>& sets) {
        if (sets.empty()) return {};
        std::vector<std::vector<int>> out;
        std::vector<int> cur(sets.size());
        cartProdRec(sets, 0, cur, out);
        return out;
    }

    // Helper for SETS macro: simply return an object the comp APIs accept.
    // We represent a "combined set" as std::vector<std::vector<int>> when needed.
    inline std::vector<std::vector<int>> makeSets(const std::initializer_list<std::vector<int>>& sets) {
        return std::vector<std::vector<int>>(sets);
    }

} // namespace mini::dsl
