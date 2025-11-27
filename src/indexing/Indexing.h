#pragma once
/*
Indexing.h
High-performance indexing and iteration system with recursive variadic templates.

Features:
- Zero-overhead range views (no memory allocation)
- Recursive variadic summation for any number of dimensions
- Compile-time optimized loops
- Uniform API for 1D, 2D, 3D, ... ND operations

Examples:
  // Create index ranges
  auto I = indices(10);           // 0..9 view
  auto R = range(5, 15);          // 5..14 view

  // Multi-dimensional summation
  GRBLinExpr total = sum([&](int i, int j, int k) {
      return cost(i,j,k) * x(i,j,k);
  }, I, J, K);

  // Constraint building
  forall([&](int i, int j) {
      model.addConstr(x(i,j) <= capacity(i));
  }, I, J);
*/

#include <vector>
#include <tuple>
#include "gurobi_c++.h"

namespace mini::dsl {

    // ============================================================================
    // ZERO-OVERHEAD RANGE VIEWS
    // ============================================================================

    /// Lightweight view of integer range [start, end)
    class RangeView {
        int start_, end_;
    public:
        RangeView(int start, int end) : start_(start), end_(end) {}

        class Iterator {
            int value_;
        public:
            Iterator(int v) : value_(v) {}
            int operator*() const { return value_; }
            Iterator& operator++() { ++value_; return *this; }
            bool operator!=(const Iterator& other) const { return value_ != other.value_; }
        };

        Iterator begin() const { return Iterator(start_); }
        Iterator end() const { return Iterator(end_); }
        int size() const { return end_ - start_; }
    };

    /// Create range [0, n) without allocation
    inline RangeView indices(int n) { return RangeView(0, n); }

    /// Create range [start, end) without allocation  
    inline RangeView range(int start, int end) { return RangeView(start, end); }

    // ============================================================================
    // RECURSIVE VARIADIC SUMMATION
    // ============================================================================

    // Expression conversion utilities
    inline GRBLinExpr toExpr(const GRBVar& v) { GRBLinExpr e = 0; e += v; return e; }
    inline GRBLinExpr toExpr(const GRBLinExpr& e) { return e; }
    inline GRBLinExpr toExpr(double d) { GRBLinExpr e = 0; e += d; return e; }
    inline GRBLinExpr toExpr(int i) { return toExpr(static_cast<double>(i)); }

    /// Recursive variadic nested-loop helper for summation
    template<typename F, typename... Ranges>
    struct SumLoop;

    /// Recursive case: at least one range remains
    template<typename F, typename Range, typename... Rest>
    struct SumLoop<F, Range, Rest...> {
        template<typename... Idxs>
        static void run(GRBLinExpr& total, F& f, const Range& range, const Rest&... rest, Idxs... idxs) {
            for (auto i : range) {
                SumLoop<F, Rest...>::run(total, f, rest..., idxs..., i);
            }
        }
    };

    /// Base case: no more ranges. Call the user lambda with collected indices
    template<typename F>
    struct SumLoop<F> {
        template<typename... Idxs>
        static void run(GRBLinExpr& total, F& f, Idxs... idxs) {
            total += toExpr(f(idxs...));
        }
    };

    /// Main summation function: sum over multiple ranges
    template<typename F, typename... Ranges>
    GRBLinExpr sum(F&& f, Ranges&&... ranges) {
        GRBLinExpr total = 0;
        SumLoop<F, Ranges...>::run(total, f, ranges...);
        return total;
    }

    // ============================================================================
    // RECURSIVE VARIADIC ITERATION
    // ============================================================================

    /// Recursive variadic nested-loop helper for iteration
    template<typename F, typename... Ranges>
    struct ForEachLoop;

    /// Recursive case: at least one range remains
    template<typename F, typename Range, typename... Rest>
    struct ForEachLoop<F, Range, Rest...> {
        template<typename... Idxs>
        static void run(F& f, const Range& range, const Rest&... rest, Idxs... idxs) {
            for (auto i : range) {
                ForEachLoop<F, Rest...>::run(f, rest..., idxs..., i);
            }
        }
    };

    /// Base case: no more ranges. Call the user lambda with collected indices
    template<typename F>
    struct ForEachLoop<F> {
        template<typename... Idxs>
        static void run(F& f, Idxs... idxs) {
            f(idxs...);
        }
    };

    /// Main iteration function: execute function over multiple ranges
    template<typename F, typename... Ranges>
    void forEach(F&& f, Ranges&&... ranges) {
        ForEachLoop<F, Ranges...>::run(f, ranges...);
    }

} // namespace mini::dsl