# API Reference

Essential classes and methods for the Mini Optimization Framework.

## Core Classes

### ModelBuilder<T, MAX>
Main framework class for building optimization models.

```cpp
template<typename T, size_t MAX = static_cast<size_t>(T::COUNT)>
class ModelBuilder {
    // Override these:
    virtual void createVariables() = 0;
    virtual void addConstraints() = 0;  
    virtual void setObjective() = 0;
    
    // Key methods:
    SolveResult solve(const RunOptions& opts = {});
    void printStats() const;
    GRBModel& getModel();
    VariableTable<T, MAX>& getVars();
};
```

**Example:**
```cpp
DECLARE_ENUM_WITH_COUNT(MyVars, X, Y)
class MyModel : public ModelBuilder<MyVars> {
    void createVariables() override {
        vars.set(MyVars::X, VariableFactory::add(model, GRB_BINARY, 0, 1, "x", 10));
    }
    // ... constraints and objective
};
```

### SolveResult
Solution information from solver.

```cpp
struct SolveResult {
    bool success;           // Solve completed without error
    int status;             // Gurobi status code
    double objective;       // Best objective value
    double runtimeSec;      // Total solve time
    double gap;             // Final optimality gap
    std::string errorMsg;   // Error description if failed
    
    bool isOptimal() const;     // Status == GRB_OPTIMAL
    bool hasSolution() const;   // Has feasible solution
};
```

### RunOptions
Solver configuration.

```cpp
struct RunOptions {
    double timeLimitSec = 0;    // 0 = no limit
    double mipGap = 0;          // 0 = solver default  
    int threads = 0;            // 0 = solver default
    bool verbose = true;        // Enable solver output
    
    // Predefined configurations:
    static RunOptions quick();      // 1min, 10% gap
    static RunOptions precise();    // 1hr, tight gap
};
```

## Variable Management

### VariableTable<EnumT, MAX>
Type-safe storage for variables.

```cpp
template<typename EnumT, size_t MAX>
class VariableTable {
    void set(EnumT key, VariableGroup&& group);
    void set(EnumT key, const GRBVar& var);
    
    VariableGroup& get(EnumT key);
    GRBVar& var(EnumT key, Indices... idx);  // Access with indices
};
```

**Example:**
```cpp
enum class Vars { X, Y, COUNT };
VariableTable<Vars, (size_t)Vars::COUNT> vars;

// Store 2D variable group
vars.set(Vars::X, VariableFactory::add(model, GRB_BINARY, 0, 1, "X", 10, 20));

// Access elements
GRBVar& x_ij = vars.var(Vars::X, i, j);  // 2D access
GRBVar& y = vars.var(Vars::Y);           // Scalar access
```

### VariableFactory
Create variables and variable groups.

```cpp
class VariableFactory {
    // Add to model
    template<typename... Sizes>
    static auto add(GRBModel& model, int vtype, double lb, double ub,
                   const std::string& name, Sizes... sizes);
    
    // Create independent handles  
    template<typename... Sizes>
    static auto create(Sizes... sizes);
};
```

**Examples:**
```cpp
// Scalar variable
GRBVar x = VariableFactory::add(model, GRB_BINARY, 0, 1, "x");

// 2D variable group
VariableGroup Y = VariableFactory::add(model, GRB_CONTINUOUS, 0, 100, "Y", 5, 10);

// Access: Y(i, j) or Y.at(i, j)
```

## Constraint Building (mini::constraint)

### Basic Constraints
```cpp
// Equality: lhs == rhs
GRBConstr addEq(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs,
                const std::string& name = "");

// Inequality: lhs <= rhs  
GRBConstr addLe(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs,
                const std::string& name = "");
```

### Logical Constraints
```cpp
// At most one variable can be true
template<typename F, typename... Ranges>
void atMostOne(GRBModel& model, F&& f, Ranges&&... ranges);

// Exactly one variable must be true  
template<typename F, typename... Ranges>
void exactlyOne(GRBModel& model, F&& f, Ranges&&... ranges);

// Logical implication: bin => (lhs <= rhs)
void implies(GRBModel& model, const GRBVar& bin, const GRBLinExpr& lhs,
             const GRBLinExpr& rhs, int value = 1);
```

### Big-M Constraints
```cpp
// bin = 1 => (lhs <= rhs)
void conBigM_Le(GRBModel& model, const GRBLinExpr& lhs, const GRBLinExpr& rhs,
                const GRBVar& bin, double M, const std::string& name = "");
```

## Indexing & Iteration (mini::dsl)

### Range Creation
```cpp
// Create range [0, n)
auto I = mini::dsl::indices(10);

// Create range [start, end)  
auto R = mini::dsl::range(5, 15);
```

### Summation & Iteration
```cpp
// Multi-dimensional summation
GRBLinExpr total = mini::dsl::sum([&](int i, int j) {
    return cost(i,j) * x(i,j);
}, I, J);

// Multi-dimensional iteration
mini::dsl::forEach([&](int i, int j) {
    model.addConstr(x(i,j) <= capacity);
}, I, J);
```

### Loop Macros
```cpp
// Traditional loops
forall(i, n) { ... }
forall2(i, I, j, J) { ... }

// Modern iteration
FORALL([&](int i, int j) { ... }, I, J);

// Quick summation  
SUM([&](int i) { return cost[i] * x[i]; }, I);
```

## Naming System (mini::naming)

```cpp
// Create names with indices
std::string name = naming::nameND("constr", i, j);  // "constr[i,j]"

// Concatenate parts
std::string name = naming::make_name("prefix_", base, "_suffix");

// Names automatically disabled in release builds (zero overhead)
```

## Common Patterns

### Assignment Constraints
```cpp
// Each task to exactly one machine
FORALL([&](int t) {
    exactlyOne(model,
        [&](int m) { return assign(t, m); },
        machines);
}, tasks);
```

### Capacity Constraints  
```cpp
// Machine capacity per period
FORALL([&](int m, int t) {
    addLe(model, 
        SUM([&](int j) { return size[j] * assign(j, m, t); }, jobs),
        capacity[m]);
}, machines, periods);
```

### Inventory Balance
```cpp
FORALL([&](int p, int t) {
    double prev = (t == 0) ? initial : inventory(p, t-1);
    addEq(model, prev + production(p,t), demand(p,t) + inventory(p,t));
}, products, periods);
```

---

*For detailed examples, see [Examples](EXAMPLES.md)*
*For advanced techniques, see [Advanced Patterns](ADVANCED.md)*