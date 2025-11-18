#pragma once
/*
ConstraintHelpers.h
Useful helpers: addEq/addLe/addGe, implies (indicator), big-M patterns,
atMostOne / exactlyOne, maxOf / minOf, printSummary.

These are small, reusable building blocks.
*/

#include <string>
#include <iostream>
#include "gurobi_c++.h"
#include "ConstraintNameND.h"

namespace mini::constraint {

    inline GRBConstr addEq(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs, const std::string& name = "") {
        return model.addConstr(lhs == rhs, name);
    }
    inline GRBConstr addLe(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs, const std::string& name = "") {
        return model.addConstr(lhs <= rhs, name);
    }
    inline GRBConstr addGe(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs, const std::string& name = "") {
        return model.addConstr(lhs >= rhs, name);
    }

    inline void addIndicator(GRBModel& model, const GRBVar& binVar, int binVal, const GRBLinExpr& lhs, const GRBLinExpr& rhs, const std::string& name = "") {
        model.addGenConstrIndicator(binVar, binVal, lhs - rhs <= 0, name);
    }

    inline void implies(GRBModel& model, const GRBVar& binVar, const GRBLinExpr& lhs, const GRBLinExpr& rhs, int val = 1, const std::string& name = "") {
        addIndicator(model, binVar, val, lhs, rhs, name);
    }

    inline void conBigM_Le(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs, const GRBVar& bin, double M, const std::string& name = "") {
        model.addConstr(lhs <= rhs + M * (1 - bin), name);
    }
    inline void conBigM_Ge(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs, const GRBVar& bin, double M, const std::string& name = "") {
        model.addConstr(lhs >= rhs - M * (1 - bin), name);
    }

    template<typename Range, typename F>
    inline void atMostOne(GRBModel& model, const Range& r, F f) {
        GRBLinExpr s = 0;
        for (auto i : r) s += f(i);
        model.addConstr(s <= 1);
    }

    template<typename Range, typename F>
    inline void exactlyOne(GRBModel& model, const Range& r, F f) {
        GRBLinExpr s = 0;
        for (auto i : r) s += f(i);
        model.addConstr(s == 1);
    }

    template<typename Range, typename F>
    inline void maxOf(GRBModel& model, GRBVar z, const Range& r, F f) {
        for (auto i : r) model.addConstr(z >= f(i));
    }

    template<typename Range, typename F>
    inline void minOf(GRBModel& model, GRBVar z, const Range& r, F f) {
        for (auto i : r) model.addConstr(z <= f(i));
    }

    inline void printSummary(GRBModel& model, std::ostream& os = std::cout) {
        os << "Vars: " << model.get(GRB_IntAttr_NumVars)
            << " Constrs: " << model.get(GRB_IntAttr_NumConstrs)
            << " NZ: " << model.get(GRB_IntAttr_NumNZs)
            << " Status: " << model.get(GRB_IntAttr_Status)
            << "\n";
    }

} // namespace mini::constraint
