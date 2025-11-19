# MiniDSL Tutorial

## Step 1: Define Your Variable Families

```cpp
enum class MyVars { X, Y, Z, MAX };
```

## Step 2: Create Model Class

```cpp
class MyModel : public mini::ModelBuilderGeneric<MyVars, 
                          static_cast<size_t>(MyVars::MAX)> {
    // Implement required methods
    void createVariables() override;
    void addConstraints() override; 
    void setObjective() override;
};
```

## Step 3: Create Variables

```cpp
void createVariables() override {
    // Scalar variable
    getVars().set(MyVars::Z, 
        VariableFactory::create(getModel(), GRB_CONTINUOUS, 0, 100, "z"));
    
    // 2D variable array
    getVars().set(MyVars::X,
        VariableFactory::create(getModel(), GRB_BINARY, 0, 1, "x", n, m));
}
```

## Step 4: Add Constraints

```cpp
void addConstraints() override {
    auto I = mini::dsl::indexSet(n);
    
    // Traditional loops
    for (int i : I) {
        mini::constraint::addLe(getModel(), 
            mini::dsl::sumOver(I, [&](int j) { return getVars().var(MyVars::X, i, j); }), 
            1, "row_sum");
    }
    
    // Using comprehensions
    auto constraints = mini::dsl::comp(mini::dsl::Index1D{I}, [&](int i) {
        GRBLinExpr sum = 0;
        for (int j : I) sum += getVars().var(MyVars::X, i, j);
        return std::make_pair(sum, 1.0);
    });
}
```

## Step 5: Set Objective and Solve

```cpp
void setObjective() override {
    getModel().setObjective(
        mini::dsl::sumOver(mini::dsl::indexSet(n), [&](int i) {
            return getVars().var(MyVars::Y, i);
        }), GRB_MINIMIZE);
}

// Solve with options
mini::RunOptions opts;
opts.timeLimitSec = 60.0;
opts.mipGap = 0.01;
auto result = solve(opts);
```

## Best Practices

1. Use enums for variable family management
2. Leverage sumOver for readable summation
3. Use constraint naming for debugging
4. Handle solve results robustly