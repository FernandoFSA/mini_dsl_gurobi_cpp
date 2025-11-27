#pragma once
/*
dsl_macros.h
Macros for common loop patterns and convenient syntax.

Features:
- Direct loop expansion for maximum performance
- Clean syntax for common multi-dimensional patterns
- Zero runtime overhead

Examples:
  // Basic loops
  forall(i, n) { ... }
  forall2(i, I, j, J) { ... }
  forall3(i, I, j, J, k, K) { ... }

  // Variadic iteration (new preferred way)
  FORALL([&](int i, int j) { ... }, I, J);
*/

#include "Indexing.h"

// ============================================================================
// ZERO-OVERHEAD LOOP MACROS (For maximum performance)
// ============================================================================

/// Single loop: forall(i, N) { ... }
#define forall(i, N) for (int i = 0, _##i##_end = int(N); i < _##i##_end; ++i)

/// Double nested loops: forall2(i, I, j, J) { ... }
#define forall2(i, I, j, J) forall(i, I) forall(j, J)

/// Triple nested loops: forall3(i, I, j, J, k, K) { ... }
#define forall3(i, I, j, J, k, K) forall2(i, I, j, J) forall(k, K)

/// Variadic iteration macro (uses recursive template)
#define FORALL(f, ...) mini::dsl::forEach(f, __VA_ARGS__)

// ============================================================================
// CONVENIENCE MACROS
// ============================================================================

/// Quick summation macro
#define SUM(f, ...) mini::dsl::sum(f, __VA_ARGS__)

/*
Convenience macro to declare an enum and append COUNT automatically.

Usage:
  DECLARE_ENUM_WITH_COUNT(Vars, X, ASSIGN, MAKESPAN)
expands to:
  enum class Vars { X, ASSIGN, MAKESPAN, COUNT };
  inline constexpr std::size_t Vars_COUNT = static_cast<std::size_t>(Vars::COUNT);
*/
#define DECLARE_ENUM_WITH_COUNT(Name, ...) \
    enum class Name { __VA_ARGS__, COUNT }; \
    inline constexpr std::size_t Name##_COUNT = static_cast<std::size_t>(Name::COUNT);