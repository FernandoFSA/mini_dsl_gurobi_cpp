#pragma once
/*
Comprehension.h
JuMP-like comprehensions:

 - comp((i) in I, f(i))         => vector<T>
 - comp((i,j) in I * J, f(i,j)) => vector<vector<T>>
 - compND(setsVec, f)           => flattened vector<T> (user can reshape)

Two helper forms:
 - SETS(I,J,...) macro maps visually to comp((i,j) in SETS(I,J), ...)
 - comp accepts the Index2D created by the rvalue product I * J

Examples in file header.
*/

#include <vector>
#include "IndexSets.h"
#include "Expr.h"

namespace mini::dsl {

    // Index binder types for comp(...) usage
    template<typename Set> struct Index1D { Set set; };
    template<typename S1, typename S2> struct Index2D { S1 s1; S2 s2; };

    // operator* to combine sets (I * J)
    template<typename S1, typename S2>
    Index2D<S1, S2> operator*(S1&& a, S2&& b) {
        return { std::forward<S1>(a), std::forward<S2>(b) };
    }

    // This overload supports comp(Index1D, f)
    template<typename Set, typename Func>
    auto comp(Index1D<Set> idx, Func&& f) {
        using T = decltype(f(std::declval<int>()));
        std::vector<T> out; out.reserve(idx.set.size());
        for (auto i : idx.set) out.push_back(f(i));
        return out;
    }

    // comp(Index2D, f) => matrix-like vector<vector<T>>
    template<typename S1, typename S2, typename Func>
    auto comp(Index2D<S1, S2> idx, Func&& f) {
        using T = decltype(f(std::declval<int>(), std::declval<int>()));
        std::vector<std::vector<T>> out; out.reserve(idx.s1.size());
        for (auto i : idx.s1) {
            std::vector<T> row; row.reserve(idx.s2.size());
            for (auto j : idx.s2) row.push_back(f(i, j));
            out.push_back(std::move(row));
        }
        return out;
    }

    // compND: flattened ND
    template<typename Func>
    auto compND(const std::vector<std::vector<int>>& sets, Func&& f) {
        using T = decltype(f(0)); // best-effort, for many cases it's fine
        std::vector<T> out;
        auto tuples = cartesianProduct(sets);
        out.reserve(tuples.size());
        for (auto& t : tuples) {
            switch (t.size()) {
            case 1: out.push_back(f(t[0])); break;
            case 2: out.push_back(f(t[0], t[1])); break;
            case 3: out.push_back(f(t[0], t[1], t[2])); break;
            default: out.push_back(f(t)); break; // user lambda must accept vector<int>
            }
        }
        return out;
    }

} // namespace mini::dsl