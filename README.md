# Mini Optimization Framework

A modern C++ mathematical optimization framework built on top of Gurobi, providing type-safe, high-performance modeling with zero-overhead abstractions.

## Features

- **Type-Safe Variables**: Enum-keyed variable tables with compile-time dimension checking
- **Multi-Dimensional Operations**: Recursive variadic templates for ND constraints and summations
- **Zero-Overhead Abstractions**: Compile-time optimized with no runtime cost
- **Clean DSL**: Mathematical syntax for constraint building and iteration
- **Memory Efficient**: Tree-based variable storage with move semantics
- **Production Ready**: Comprehensive error handling and solver configuration

## Quick Example

```cpp
#include "core/ModelBuilder.h"
#include "indexing/dsl_macros.h"

DECLARE_ENUM_WITH_COUNT(Vars, X, ASSIGN, MAKESPAN)

class ProductionModel : public mini::ModelBuilder<Vars> {
protected:
    void createVariables() override {
        // 3D assignment variables
        vars.set(Vars::ASSIGN, 
            mini::VariableFactory::add(model, GRB_BINARY, 0, 1, "assign", 10, 5, 8));
        
        // Scalar makespan variable
        vars.set(Vars::MAKESPAN, 
            mini::VariableFactory::add(model, GRB_CONTINUOUS, 0, 1000, "makespan"));
    }

    void addConstraints() override {
        auto T = mini::dsl::indices(10);  // Tasks
        auto M = mini::dsl::indices(5);   // Machines
        auto S = mini::dsl::indices(8);   // Time slots

        // Each task assigned to exactly one machine and time slot
        FORALL([&](int t) {
            mini::constraint::exactlyOne(model,
                [&](int m, int s) { return vars.var(Vars::ASSIGN, t, m, s); },
                M, S);
        }, T);
    }

    void setObjective() override {
        model.setObjective(vars.var(Vars::MAKESPAN), GRB_MINIMIZE);
    }
};

int main() {
    ProductionModel model;
    auto result = model.solve({.timeLimitSec = 300, .mipGap = 0.01});
    
    if (result.success) {
        std::cout << "Optimal makespan: " << result.objective << "\n";
    }
    return 0;
}
```

## Installation

### Prerequisites
- Gurobi Optimizer 10.0+ with C++ bindings
- C++20 compatible compiler (GCC 7+, Clang 5+, MSVC 2019+)
- CMake 3.15+

### Basic Setup
1. Clone the repository:
```bash
git clone https://github.com/your-org/mini-optimization-framework
cd mini-optimization-framework
```

2. Configure with CMake:
```bash
mkdir build && cd build
cmake .. -DGUROBI_HOME=/path/to/gurobi -DGUROBI_LIBRARY=/path/to/gurobi_c++.a
make -j4
```

3. Include in your project:
```cmake
target_link_libraries(your_project mini_optimization_framework)
```

## Key Components

### Model Building
- **ModelBuilder**: Template-based framework for structured model development
- **VariableTable**: Type-safe storage with enum keys
- **RunOptions**: Comprehensive solver configuration

### Variable Management
- **VariableFactory**: Create scalar and ND variables with automatic naming
- **VariableGroup**: Recursive tree structure for efficient ND storage

### Domain-Specific Language
- **Indexing**: Zero-overhead range views and variadic iteration
- **ConstraintBuilders**: High-level patterns (atMostOne, exactlyOne, bigM)
- **Naming**: Compile-time controlled naming system

## Performance

The framework is designed for maximum performance:
- **Zero runtime overhead** for abstraction layers
- **Compile-time optimization** with constexpr and templates
- **Memory-efficient** tree structures with move semantics
- **Direct Gurobi integration** without intermediate layers

```cpp
// Compiles to equivalent raw Gurobi calls - zero overhead!
auto total = SUM([&](int i, int j) { 
    return cost(i,j) * x(i,j); 
}, I, J);
```

## Documentation

- [Getting Started](docs/GETTING_STARTED.md) - Installation and first model
- [Examples](docs/EXAMPLES.md) - Practical model implementations
- [API Reference](docs/API_REFERENCE.md) - Detailed class documentation
- [Advanced Patterns](docs/ADVANCED.md) - Complex modeling techniques

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

- 🐛 [Issues](https://github.com/FernandoFSA/mini_dsl_gurobi_cpp/issues)