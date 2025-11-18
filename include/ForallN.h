#pragma once
/*
ForallN.h
Macros for common loop idioms:
  forall(i, N)            => for(i = 0; i < N; ++i)
  forall2(i,I,j,J)        => nested 2 loops
  forall3(i,I,j,J,k,K)    => nested 3 loops
  forallND(t, sets)       => iterate over cartesianProduct(sets) where t is the tuple vector<int>

Usage:
  forall(i, n) { ... }
  forall2(i,I,j,J) { ... }
  forallND(t, {I,J,K}) { int i = t[0]; int j = t[1]; ... }
*/

#define forall(i, N) for (int i = 0; i < int(N); ++i)
#define forall2(i, I, j, J) for (int i = 0; i < int(I); ++i) for (int j = 0; j < int(J); ++j)
#define forall3(i, I, j, J, k, K) for (int i = 0; i < int(I); ++i) for (int j = 0; j < int(J); ++j) for (int k = 0; k < int(K); ++k)
#define forallND(tuple, sets) for (const auto& tuple : mini::dsl::cartesianProduct(sets))
