#pragma once
/*
dsl_macros.h
- Lightweight macro helpers for the DSL.
- WARNING: '#define in <' replaces the token 'in' globally within translation units.
  Avoid using identifiers named 'in' in files that include this header.

Usage:
  #include "dsl_macros.h"
  using namespace mini::dsl;

  auto I = indexSet(n);
  comp((i,j) in I * J, ...);      // recommended
  comp((i,j) in SETS(I,J), ...);  // alternative AMPL-like visual syntax

Defines:
  - in      : macro to make 'i in I' parse as 'i < I' (required by IndexBinder).
  - SETS(...) : macro to expand a tuple-like (I,J,...) into a helper that comp() accepts.
*/

#define in <

/* SETS(...) accepts a list of set names and expands to a specially typed helper.
   Example: comp((i,j) in SETS(I,J), f)  -> equivalent to comp((i,j) in I * J, f)
   We implement SETS to expand to a `mini::dsl::IndexND` initializer. */
#define SETS(...) mini::dsl::makeSets({ __VA_ARGS__ })