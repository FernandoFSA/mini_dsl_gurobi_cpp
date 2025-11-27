# Getting Started

Build your first optimization model in 10 minutes.

## Installation

### 1. Prerequisites
- Gurobi Optimizer 10.0+ with C++ bindings
- C++20 compiler
- CMake 3.15+

### 2. Quick Setup
```bash
git clone https://github.com/FernandoFSA/mini_dsl_gurobi_cpp.git
cd mini_dsl_gurobi_cpp
mkdir build && cd build
cmake .. -DGUROBI_HOME=/path/to/gurobi
make -j4
```

### 3. Add to Your Project
```cmake
target_link_libraries(your_project mini_optimization_framework)
```

## Your First Model: Knapsack Problem

```cpp
#include "core/ModelBuilder.h"
#include "core/ConstraintBuilders.h"

// 1. Define variables using enum
DECLARE_ENUM_WITH_COUNT(KnapsackVars, SELECT, TOTAL_VALUE)

class KnapsackModel : public mini::ModelBuilder<KnapsackVars> {
private:
    std::vector<double> values = {60, 100, 120};
    std::vector<double> weights = {10, 20, 30};
    double capacity = 50.0;
    
public:
    void createVariables() override {
        // Create binary selection variables
        vars.set(KnapsackVars::SELECT,
            mini::VariableFactory::add(model, GRB_BINARY, 0, 1, "select", values.size()));
    }

    void addConstraints() override {
        auto Items = mini::dsl::indices(values.size());
        
        // Capacity constraint
        GRBLinExpr totalWeight = mini::dsl::sum([&](int i) {
            return weights[i] * vars.var(KnapsackVars::SELECT, i);
        }, Items);
        
        mini::constraint::addLe(model, totalWeight, capacity, "capacity");
    }

    void setObjective() override {
        auto Items = mini::dsl::indices(values.size());
        
        // Maximize total value
        GRBLinExpr totalValue = mini::dsl::sum([&](int i) {
            return values[i] * vars.var(KnapsackVars::SELECT, i);
        }, Items);
        
        model.setObjective(totalValue, GRB_MAXIMIZE);
    }
};

int main() {
    KnapsackModel model;
    auto result = model.solve();
    
    if (result.success) {
        std::cout << "Optimal value: " << result.objective << "\n";
    }
    return 0;
}
```

## Core Concepts

### 1. Variable Types
```cpp
// Scalar variable
vars.set(Vars::X, VariableFactory::add(model, GRB_CONTINUOUS, 0, 100, "x"));

// 2D variable array  
vars.set(Vars::Y, VariableFactory::add(model, GRB_BINARY, 0, 1, "y", 10, 20));

// Access elements
GRBVar& y_ij = vars.var(Vars::Y, i, j);
```

### 2. Common Constraints
```cpp
// Basic constraints
mini::constraint::addLe(model, x + y, 10, "sum_constraint");
mini::constraint::addEq(model, total, demand, "balance");

// Logical constraints
mini::constraint::exactlyOne(model, 
    [&](int i) { return assign[i]; }, indices(n));

// Big-M constraints
mini::constraint::conBigM_Le(model, production, capacity, activate, 1000);
```

### 3. Solving & Configuration
```cpp
// Basic solve
auto result = model.solve();

// Configured solve
mini::RunOptions options;
options.timeLimitSec = 300;
options.mipGap = 0.01;
options.verbose = true;

auto result = model.solve(options);

// Check results
if (result.isOptimal()) {
    std::cout << "Optimal solution found!\n";
}
```

## Next Steps
- [Examples](EXAMPLES.md) - Practical model implementations
- [API Reference](API_REFERENCE.md) - Detailed class documentation
- [Advanced Patterns](ADVANCED.md) - Complex modeling techniques
[file content end]