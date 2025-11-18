#pragma once
/*
Expr.h
Lightweight wrapper `mini::dsl::expr` around GRBLinExpr to enable
algebraic operators and to produce constraint pairs (lhs-rhs, op).

Usage:
  using mini::dsl::expr;
  expr e = vars.var(...);
  auto c = (e + 2*expr(3)) <= expr(10);
  mini::dsl::constr(model, c, "name");
*/

#include "gurobi_c++.h"
#include <utility>

namespace mini::dsl {

    class expr {
        GRBLinExpr e;
    public:
        expr() : e(0.0) {}
        expr(double v) : e(v) {}
        expr(const GRBLinExpr& x) : e(x) {}
        expr(const GRBVar& v) { e = 0; e += v; }
        operator GRBLinExpr() const { return e; }
        GRBLinExpr& raw() { return e; }
        const GRBLinExpr& raw() const { return e; }
        expr operator-() const { return expr(-e); }
        expr operator+(const expr& o) const { return expr(e + o.e); }
        expr operator-(const expr& o) const { return expr(e - o.e); }
        expr operator*(double k) const { return expr(e * k); }
        expr operator/(double k) const { return expr(e / k); }

        friend expr operator*(double k, const expr& x) { return expr(k * x.e); }
        friend expr operator+(double k, const expr& x) { return expr(k + x.e); }

        friend std::pair<GRBLinExpr, char> operator<=(const expr& lhs, const expr& rhs) {
            return { lhs.e - rhs.e, '<' };
        }
        friend std::pair<GRBLinExpr, char> operator>=(const expr& lhs, const expr& rhs) {
            return { lhs.e - rhs.e, '>' };
        }
        friend std::pair<GRBLinExpr, char> operator==(const expr& lhs, const expr& rhs) {
            return { lhs.e - rhs.e, '=' };
        }
    };

    inline GRBConstr constr(GRBModel& model, const std::pair<GRBLinExpr, char>& c, const std::string& name = "") {
        auto [expr, op] = c;
        if (op == '<') return model.addConstr(expr <= 0, name);
        if (op == '>') return model.addConstr(expr >= 0, name);
        return model.addConstr(expr == 0, name);
    }

} // namespace mini::dsl
