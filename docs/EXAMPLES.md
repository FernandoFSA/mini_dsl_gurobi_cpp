# Examples

Practical optimization models using the Mini Optimization Framework.

## Production Planning

Multi-period production with inventory and capacity constraints.

```cpp
#include "core/ModelBuilder.h"

DECLARE_ENUM_WITH_COUNT(ProductionVars, PRODUCE, INVENTORY, SETUP)

class ProductionModel : public mini::ModelBuilder<ProductionVars> {
private:
    int numProducts = 3;
    int numPeriods = 6;
    std::vector<std::vector<double>> demand;
    std::vector<double> capacity = {100, 120, 90};
    
public:
    void createVariables() override {
        vars.set(ProductionVars::PRODUCE,
            mini::VariableFactory::add(model, GRB_CONTINUOUS, 0, 150, "produce", 
                numProducts, numPeriods));
        
        vars.set(ProductionVars::INVENTORY,
            mini::VariableFactory::add(model, GRB_CONTINUOUS, 0, 200, "inventory",
                numProducts, numPeriods));
        
        vars.set(ProductionVars::SETUP,
            mini::VariableFactory::add(model, GRB_BINARY, 0, 1, "setup",
                numProducts, numPeriods));
    }

    void addConstraints() override {
        auto P = mini::dsl::indices(numProducts);
        auto T = mini::dsl::indices(numPeriods);
        
        // Capacity constraints
        FORALL([&](int p, int t) {
            mini::constraint::conBigM_Le(model,
                vars.var(ProductionVars::PRODUCE, p, t),
                vars.var(ProductionVars::SETUP, p, t),
                vars.var(ProductionVars::SETUP, p, t),
                1000, "setup_force");
        }, P, T);
        
        // Inventory balance
        FORALL([&](int p, int t) {
            double prev = (t == 0) ? 10.0 : 0; // Initial inventory
            GRBLinExpr prevInventory = (t == 0) ? prev : 
                vars.var(ProductionVars::INVENTORY, p, t-1);
                
            mini::constraint::addEq(model,
                prevInventory + vars.var(ProductionVars::PRODUCE, p, t),
                demand[p][t] + vars.var(ProductionVars::INVENTORY, p, t),
                "inventory_balance");
        }, P, T);
    }

    void setObjective() override {
        auto P = mini::dsl::indices(numProducts);
        auto T = mini::dsl::indices(numPeriods);
        
        GRBLinExpr cost = mini::dsl::sum([&](int p, int t) {
            return 2.0 * vars.var(ProductionVars::PRODUCE, p, t) +
                   0.5 * vars.var(ProductionVars::INVENTORY, p, t) +
                   100 * vars.var(ProductionVars::SETUP, p, t);
        }, P, T);
        
        model.setObjective(cost, GRB_MINIMIZE);
    }
};
```

## Facility Location

Choose facilities to open and assign customers at minimum cost.

```cpp
DECLARE_ENUM_WITH_COUNT(FacilityVars, OPEN, ASSIGN)

class FacilityLocation : public mini::ModelBuilder<FacilityVars> {
private:
    int numFacilities = 5;
    int numCustomers = 20;
    std::vector<double> fixedCosts = {100, 150, 120, 180, 130};
    std::vector<std::vector<double>> serviceCost;
    
public:
    void createVariables() override {
        vars.set(FacilityVars::OPEN,
            mini::VariableFactory::add(model, GRB_BINARY, 0, 1, "open", numFacilities));
        
        vars.set(FacilityVars::ASSIGN,
            mini::VariableFactory::add(model, GRB_BINARY, 0, 1, "assign",
                numFacilities, numCustomers));
    }

    void addConstraints() override {
        auto F = mini::dsl::indices(numFacilities);
        auto C = mini::dsl::indices(numCustomers);
        
        // Customer assignment
        FORALL([&](int c) {
            mini::constraint::exactlyOne(model,
                [&](int f) { return vars.var(FacilityVars::ASSIGN, f, c); },
                F);
        }, C);
        
        // Facility must be open to serve customers
        FORALL([&](int f, int c) {
            mini::constraint::implies(model,
                vars.var(FacilityVars::ASSIGN, f, c),
                vars.var(FacilityVars::OPEN, f),
                1, "open_if_serve");
        }, F, C);
    }

    void setObjective() override {
        auto F = mini::dsl::indices(numFacilities);
        auto C = mini::dsl::indices(numCustomers);
        
        GRBLinExpr cost = mini::dsl::sum([&](int f) {
            return fixedCosts[f] * vars.var(FacilityVars::OPEN, f);
        }, F);
        
        cost += mini::dsl::sum([&](int f, int c) {
            return serviceCost[f][c] * vars.var(FacilityVars::ASSIGN, f, c);
        }, F, C);
        
        model.setObjective(cost, GRB_MINIMIZE);
    }
};
```

## Portfolio Optimization

Mean-variance portfolio optimization with constraints.

```cpp
DECLARE_ENUM_WITH_COUNT(PortfolioVars, WEIGHT)

class PortfolioOptimizer : public mini::ModelBuilder<PortfolioVars> {
private:
    int numAssets = 30;
    std::vector<double> expectedReturns;
    std::vector<std::vector<double>> covariance;
    
public:
    void createVariables() override {
        vars.set(PortfolioVars::WEIGHT,
            mini::VariableFactory::add(model, GRB_CONTINUOUS, 0, 0.2, "weight", numAssets));
    }

    void addConstraints() override {
        auto A = mini::dsl::indices(numAssets);
        
        // Fully invested
        GRBLinExpr totalWeight = mini::dsl::sum([&](int i) {
            return vars.var(PortfolioVars::WEIGHT, i);
        }, A);
        
        mini::constraint::addEq(model, totalWeight, 1.0, "fully_invested");
    }

    void setObjective() override {
        auto A = mini::dsl::indices(numAssets);
        
        // Expected return
        GRBLinExpr expectedReturn = mini::dsl::sum([&](int i) {
            return expectedReturns[i] * vars.var(PortfolioVars::WEIGHT, i);
        }, A);
        
        // Risk (variance)
        GRBQuadExpr risk = 0;
        FORALL([&](int i, int j) {
            risk += covariance[i][j] * vars.var(PortfolioVars::WEIGHT, i) 
                          * vars.var(PortfolioVars::WEIGHT, j);
        }, A, A);
        
        // Mean-variance objective
        model.setObjective(expectedReturn - risk, GRB_MAXIMIZE);
    }
};
```

## Vehicle Routing (Simplified)

Basic vehicle routing with capacity constraints.

```cpp
DECLARE_ENUM_WITH_COUNT(VRPVars, VISIT)

class VehicleRouting : public mini::ModelBuilder<VRPVars> {
private:
    int numCustomers = 10;
    int numVehicles = 3;
    std::vector<double> demands;
    double vehicleCapacity = 50;
    
public:
    void createVariables() override {
        vars.set(VRPVars::VISIT,
            mini::VariableFactory::add(model, GRB_BINARY, 0, 1, "visit",
                numVehicles, numCustomers, numCustomers));
    }

    void addConstraints() override {
        auto V = mini::dsl::indices(numVehicles);
        auto C = mini::dsl::indices(numCustomers);
        
        // Each customer served exactly once
        FORALL([&](int i) {
            GRBLinExpr totalVisits = mini::dsl::sum([&](int k, int j) {
                return vars.var(VRPVars::VISIT, k, i, j);
            }, V, C);
            mini::constraint::addEq(model, totalVisits, 1, "serve_once");
        }, C);
        
        // Vehicle capacity
        FORALL([&](int k) {
            GRBLinExpr totalDemand = mini::dsl::sum([&](int i, int j) {
                return demands[i] * vars.var(VRPVars::VISIT, k, i, j);
            }, C, C);
            mini::constraint::addLe(model, totalDemand, vehicleCapacity, "capacity");
        }, V);
    }

    void setObjective() override {
        // Minimize total distance (simplified)
        auto V = mini::dsl::indices(numVehicles);
        auto C = mini::dsl::indices(numCustomers);
        
        GRBLinExpr distance = mini::dsl::sum([&](int k, int i, int j) {
            return computeDistance(i, j) * vars.var(VRPVars::VISIT, k, i, j);
        }, V, C, C);
        
        model.setObjective(distance, GRB_MINIMIZE);
    }
};
```

## Usage Tips

### 1. Solver Configuration
```cpp
mini::RunOptions options;
options.timeLimitSec = 300;    // 5 minutes
options.mipGap = 0.01;         // 1% optimality gap
options.threads = 4;           // Use 4 threads

auto result = model.solve(options);
```

### 2. Model Analysis
```cpp
model.printStats();  // Print variable/constraint counts

if (result.success) {
    std::cout << "Objective: " << result.objective << "\n";
    std::cout << "Solve time: " << result.runtimeSec << "s\n";
    std::cout << "Optimality gap: " << result.gap * 100 << "%\n";
}
```

### 3. Performance Tips
- Use `dsl::sum` for efficient constraint building
- Set tight variable bounds when possible
- Use appropriate variable types (binary vs continuous)
- Enable presolve: `model.set(GRB_IntParam_Presolve, 2)`

---

*Ready for advanced techniques? See [Advanced Patterns](ADVANCED.md)*