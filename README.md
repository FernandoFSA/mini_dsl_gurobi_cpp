# MiniDSL: C++ Optimization DSL for Gurobi

A high-performance, expressive Domain Specific Language for mathematical optimization built on Gurobi solver.

## 🚀 Features

- **JuMP-like Syntax**: Familiar mathematical modeling syntax
- **Type-Safe**: Compile-time checking with templates
- **High-Performance**: Zero-cost abstractions
- **Multi-dimensional Variables**: Natural N-dimensional array support
- **Comprehensions**: Collection operations similar to JuMP
- **Enum-based Management**: Organized variable families
    
## 📁 Architecture

#### 1. **Variable Management:** *Variable creation with automatic naming*

- `VariableGroup.h` - N-dimensional variable containers
- `VariableTable.h` - Enum-based variable family storage
- `VariableFactory.h` - Variable creation with automatic naming

#### 2. **Model Building:** *Base class for model definition*

- `ModelBuilderGeneric.h` - Templated model orchestration
- `Expr.h` - Algebraic expression wrapper for GRBLinExpr

#### 3. **Indexing & Sets:** *Functional-style collection operations*

- `IndexSets.h` - Index set creation and cartesian products
- `Comprehension.h` - JuMP-like comprehensions for collections

#### 2. **Constraint Building:** *Common constraint patterns*

- `ConstraintHelpers.h` - Common constraint patterns
- `ConstraintNameND.h` - Multi-dimensional constraint naming
- `SumOver.h` - Summation utilities

#### 3. **Syntax Sugar**

- `dsl_macros.h` - Macros for readable syntax
- `ForallN.h` - Loop macros for common patterns

## 📋 Requirements

- Gurobi Optimizer 10.0+

- C++17 compatible compiler

- CMake 3.15+ (recommended)

## 🚀 Usage Instructions

1. Copy all these files into your project directory alongside your existing header files
2. Make the build script executable: chmod +x build.sh
3. Run the build: ./build.sh
4. The examples will compile and run automatically

#### The build script will:

✅ Detect your Gurobi installation

✅ Configure CMake automatically

✅ Build both examples

✅ Run the examples to verify everything works


## 📦 Quick Start

```cpp
#include "mini_dsl.h"

// Define your variable families
enum class Vars { X, Y, MAKESPAN, MAX };

class MyModel : public mini::ModelBuilderGeneric<Vars, (size_t)Vars::MAX> {
public:
    int n = 10;
    
    void createVariables() override {
        getVars().set(Vars::X, 
            VariableFactory::create(getModel(), GRB_BINARY, 0, 1, "X", n, n));
        getVars().set(Vars::Y, 
            VariableFactory::create(getModel(), GRB_CONTINUOUS, 0, GRB_INFINITY, "Y", n));
    }
    
    void addConstraints() override {
        auto I = mini::dsl::indexSet(n);
        for (int i : I) {
            addLe(getModel(), sumOver(I, [&](int j) { 
                return getVars().var(Vars::X, i, j); 
            }), 1, "row_sum");
        }
    }
    
    void setObjective() override {
        getModel().setObjective(
            sumOver(mini::dsl::indexSet(n), [&](int i) {
                return getVars().var(Vars::Y, i);
            }), GRB_MINIMIZE);
    }
};

int main() {
    MyModel model;
    auto result = model.solve({{.timeLimitSec = 60, .verbose = true}});
    if (result.success) {
        std::cout << "Objective: " << result.objective << std::endl;
    }
    return 0;
}
```