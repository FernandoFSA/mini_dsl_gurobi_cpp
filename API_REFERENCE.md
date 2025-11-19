# API Reference

## Core Classes

### ModelBuilderGeneric<EnumT, MAX>
Main model building class.

**Key Methods:**
- `createVariables()` - Pure virtual, define variables
- `addConstraints()` - Pure virtual, add constraints  
- `setObjective()` - Pure virtual, set objective
- `solve(opts)` - Build and solve model
- `getVars()` - Access variable table

### VariableTable<EnumT, MAX>
Enum-indexed variable storage.

**Methods:**
- `set(enum, VariableGroup)` - Store variable group
- `var(enum, indices...)` - Access variable

### VariableFactory
Variable creation utilities.

**Static Methods:**
- `create(model, type, lb, ub, name, dims...)` - Create attached variables
- `createIndependent(dims...)` - Create independent variables

## DSL Components

### Comprehensions
```cpp
auto I = indexSet(5);
auto values = comp(Index1D{I}, [](int i) { return i*i; });
auto matrix = comp(Index2D{I, I}, [](int i, int j) { return i+j; });
```

## Summations

```cpp
auto total = sumOver(I, [](int i) { return x[i]; });
auto matrix_sum = sumOver(I, J, [](int i, int j) { return A[i][j]; });
```

## Constraint Helpers

```cpp
addEq(model, lhs, rhs, name);
addLe(model, lhs, rhs, name); 
addGe(model, lhs, rhs, name);
exactlyOne(model, range, lambda);
atMostOne(model, range, lambda);
```

## Expression System

### expr wrapper

```cpp
using mini::dsl::expr;
expr e = x + 2*y;
auto constraint = (e <= 10);
constr(model, constraint, "name");
```

## Run Options

```cpp
mini::RunOptions opts;
opts.timeLimitSec = 60.0;
opts.mipGap = 0.01;
opts.threads = 4;
opts.verbose = true;
```

## Solve Result

```cpp
struct SolveResult {
    bool success;
    int status;
    double objective;
    double runtimeSec;
    std::string errorMsg;
};
```