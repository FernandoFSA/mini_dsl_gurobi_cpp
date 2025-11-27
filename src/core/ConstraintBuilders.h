#pragma once
/*
ConstraintBuilders.h
High-level constraint building patterns using recursive variadic templates.

Features:
- Common constraint patterns (atMostOne, exactlyOne, big-M)
- Variadic iteration for constraint building
- Logical implications and indicator constraints
- Clean, reusable building blocks

Examples:
  // Basic constraints
  addEq(model, x + y, 1, "balance");

  // Multi-dimensional constraints
  FORALL([&](int i, int j, int k) {
      addLe(model, x(i,j,k), capacity(k), "capacity_constraint");
  }, I, J, K);

  // Logical constraints
  atMostOne(model, I, [&](int i) { return x(i); });
  exactlyOne(model, I, J, [&](int i, int j) { return assign(i,j); });
*/

#include <string>
#include <iostream>
#include "gurobi_c++.h"
#include "../indexing/Indexing.h"

namespace mini::constraint {

    // ============================================================================
    // BASIC CONSTRAINT HELPERS
    // ============================================================================

    /// Add equality constraint: lhs == rhs
    inline GRBConstr addEq(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs,
        const std::string& name = "") {
        return model.addConstr(lhs == rhs, name);
    }

    /// Add less-than-or-equal constraint: lhs <= rhs  
    inline GRBConstr addLe(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs,
        const std::string& name = "") {
        return model.addConstr(lhs <= rhs, name);
    }

    /// Add greater-than-or-equal constraint: lhs >= rhs
    inline GRBConstr addGe(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs,
        const std::string& name = "") {
        return model.addConstr(lhs >= rhs, name);
    }

    // ============================================================================
    // VARIADIC CONSTRAINT BUILDING
    // ============================================================================

    /// Add constraints over multiple dimensions
    template<typename F, typename... Ranges>
    void addConstr(GRBModel& model, F&& f, Ranges&&... ranges) {
        dsl::forEach([&](auto... idx) {
            model.addConstr(f(idx...));
            }, ranges...);
    }

    /// Add constraints with names over multiple dimensions
    template<typename F, typename... Ranges>
    void addConstr(GRBModel& model, F&& f, const std::string& baseName, Ranges&&... ranges) {
        dsl::forEach([&](auto... idx) {
            std::string name = naming::nameND(baseName, idx...);
            model.addConstr(f(idx...), name);
            }, ranges...);
    }

    // ============================================================================
    // LOGICAL AND INDICATOR CONSTRAINTS
    // ============================================================================

    /// Add indicator constraint: binVar = value => (lhs <= rhs)
    inline void addIndicator(GRBModel& model, const GRBVar& binVar, int value,
        const GRBLinExpr& lhs, const GRBLinExpr& rhs,
        const std::string& name = "") {
        model.addGenConstrIndicator(binVar, value, lhs - rhs <= 0, name);
    }

    /// Logical implication: binVar => (lhs <= rhs)
    inline void implies(GRBModel& model, const GRBVar& binVar, const GRBLinExpr& lhs,
        const GRBLinExpr& rhs, int value = 1, const std::string& name = "") {
        addIndicator(model, binVar, value, lhs, rhs, name);
    }

    // ============================================================================
    // CARDINALITY CONSTRAINTS (Variadic versions)
    // ============================================================================

    /// At most one variable in the set can be true (multi-dimensional)
    template<typename F, typename... Ranges>
    void atMostOne(GRBModel& model, F&& f, Ranges&&... ranges) {
        GRBLinExpr sumExpr = dsl::sum(f, ranges...);
        model.addConstr(sumExpr <= 1);
    }

    /// Exactly one variable in the set must be true (multi-dimensional)
    template<typename F, typename... Ranges>
    void exactlyOne(GRBModel& model, F&& f, Ranges&&... ranges) {
        GRBLinExpr sumExpr = dsl::sum(f, ranges...);
        model.addConstr(sumExpr == 1);
    }

    // ============================================================================
    // BIG-M CONSTRAINTS
    // ============================================================================

    /// Big-M constraint for implication: bin = 1 => (lhs <= rhs)
    inline void conBigM_Le(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs,
        const GRBVar& bin, double M, const std::string& name = "") {
        model.addConstr(lhs <= rhs + M * (1 - bin), name);
    }

    /// Big-M constraint for implication: bin = 1 => (lhs >= rhs)
    inline void conBigM_Ge(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs,
        const GRBVar& bin, double M, const std::string& name = "") {
        model.addConstr(lhs >= rhs - M * (1 - bin), name);
    }

    // ============================================================================
    // MIN/MAX RELATIONSHIPS (Variadic versions)
    // ============================================================================

    /// Force z to be at least the maximum of f(i,j,...) over ranges
    template<typename F, typename... Ranges>
    void maxOf(GRBModel& model, GRBVar z, F&& f, Ranges&&... ranges) {
        dsl::forEach([&](auto... idx) {
            model.addConstr(z >= f(idx...));
            }, ranges...);
    }

    /// Force z to be at most the minimum of f(i,j,...) over ranges
    template<typename F, typename... Ranges>
    void minOf(GRBModel& model, GRBVar z, F&& f, Ranges&&... ranges) {
        dsl::forEach([&](auto... idx) {
            model.addConstr(z <= f(idx...));
            }, ranges...);
    }

} // namespace mini::constraint