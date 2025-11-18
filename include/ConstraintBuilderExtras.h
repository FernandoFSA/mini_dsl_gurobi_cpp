#pragma once
/*
ConstraintBuilderExtras.h
- Masked-sum sugar: sum((i) in I, f) and variants
- Helpers for sumOver with Index binder types
- Examples of usage
*/

#include "Comprehension.h"
#include "SumOver.h"

namespace mini::dsl {

    // Masked sum: sum(i in I, f)
    template<typename Set, typename F>
    GRBLinExpr ms_sum(Index1D<Set> idx, F f) {
        return sumOver(idx.set, f);
    }

    // Masked 2D sum: sum((i,j) in I*J, f)
    template<typename S1, typename S2, typename F>
    GRBLinExpr ms_sum(Index2D<S1, S2> idx, F f) {
        return sumOver(idx.s1, idx.s2, f);
    }

    // Variadic ND is done via sumOver(setsVec, lambda)
} // namespace mini::dsl