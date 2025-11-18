#pragma once
/*
SumOver.h
Provides:
 - sumOver(range, f)      // 1D
 - sumOver(r1, r2, f)     // 2D
 - sumOver(setsVec, f)    // ND using cartesianProduct
 - sum(n, lambda) fast path

f may return GRBVar, GRBLinExpr, int, double. We convert via toExpr.
*/

#include "IndexSets.h"
#include "VariableTable.h"
#include "Expr.h" // uses toExpr
#include <vector>

namespace mini::dsl {

    inline GRBLinExpr toExpr(const GRBVar& v) { GRBLinExpr e = 0; e += v; return e; }
    inline GRBLinExpr toExpr(const GRBLinExpr& e) { return e; }
    inline GRBLinExpr toExpr(double d) { GRBLinExpr e = 0; e += d; return e; }
    inline GRBLinExpr toExpr(int i) { return toExpr(static_cast<double>(i)); }

    // fast path
    template<typename F, typename IndexType>
    GRBLinExpr sum(IndexType n, F f) {
        GRBLinExpr total = 0;
        for (IndexType i = 0; i < n; ++i) total += toExpr(f(i));
        return total;
    }

    template<typename Range, typename F>
    GRBLinExpr sumOver(const Range& r, F f) {
        GRBLinExpr total = 0;
        for (auto i : r) total += toExpr(f(i));
        return total;
    }

    template<typename R1, typename R2, typename F>
    GRBLinExpr sumOver(const R1& r1, const R2& r2, F f) {
        GRBLinExpr total = 0;
        for (auto i : r1) for (auto j : r2) total += toExpr(f(i, j));
        return total;
    }

    template<typename F>
    GRBLinExpr sumOver(const std::vector<std::vector<int>>& sets, F f) {
        GRBLinExpr total = 0;
        auto tuples = mini::dsl::cartesianProduct(sets);
        for (const auto& t : tuples) {
            switch (t.size()) {
            case 1: total += toExpr(f(t[0])); break;
            case 2: total += toExpr(f(t[0], t[1])); break;
            case 3: total += toExpr(f(t[0], t[1], t[2])); break;
            default:
                // Fallback: user should provide a lambda accepting vector<int> for >3D
                total += toExpr(f(t));
            }
        }
        return total;
    }

} // namespace mini::dsl