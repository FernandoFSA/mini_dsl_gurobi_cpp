# Advanced Patterns

Key advanced modeling techniques for complex optimization problems.

## Piecewise Linear Functions

Model non-linear relationships using SOS2 constraints.

```cpp
DECLARE_ENUM_WITH_COUNT(PiecewiseVars, X, LAMBDA, SEGMENT)

class PiecewiseLinearModel : public mini::ModelBuilder<PiecewiseVars> {
private:
    std::vector<double> breakpoints = {0, 2, 4, 6, 8, 10};
    std::vector<double> values = {0, 4, 16, 36, 64, 100}; // f(x) = x²
    
public:
    void createVariables() override {
        vars.set(PiecewiseVars::X,
            mini::VariableFactory::add(model, GRB_CONTINUOUS, 0, 10, "x"));
        
        vars.set(PiecewiseVars::LAMBDA,
            mini::VariableFactory::add(model, GRB_CONTINUOUS, 0, 1, "lambda", 
                breakpoints.size()));
        
        vars.set(PiecewiseVars::SEGMENT,
            mini::VariableFactory::add(model, GRB_BINARY, 0, 1, "segment", 
                breakpoints.size() - 1));
    }

    void addConstraints() override {
        auto Points = mini::dsl::indices(breakpoints.size());
        auto Segments = mini::dsl::indices(breakpoints.size() - 1);
        
        // Convex combination
        GRBLinExpr lambdaSum = mini::dsl::sum([&](int i) {
            return vars.var(PiecewiseVars::LAMBDA, i);
        }, Points);
        mini::constraint::addEq(model, lambdaSum, 1, "lambda_sum");
        
        // X as convex combination
        GRBLinExpr xExpr = mini::dsl::sum([&](int i) {
            return breakpoints[i] * vars.var(PiecewiseVars::LAMBDA, i);
        }, Points);
        mini::constraint::addEq(model, vars.var(PiecewiseVars::X), xExpr, "x_def");
        
        // SOS2 constraint using binary variables
        FORALL([&](int s) {
            GRBLinExpr segmentLambdas = vars.var(PiecewiseVars::LAMBDA, s) + 
                                       vars.var(PiecewiseVars::LAMBDA, s+1);
            mini::constraint::conBigM_Le(model, segmentLambdas, 
                vars.var(PiecewiseVars::SEGMENT, s), 
                vars.var(PiecewiseVars::SEGMENT, s), 2.0);
        }, Segments);
        
        mini::constraint::exactlyOne(model,
            [&](int s) { return vars.var(PiecewiseVars::SEGMENT, s); }, Segments);
    }

    void setObjective() override {
        auto Points = mini::dsl::indices(breakpoints.size());
        
        GRBLinExpr f_x = mini::dsl::sum([&](int i) {
            return values[i] * vars.var(PiecewiseVars::LAMBDA, i);
        }, Points);
        
        model.setObjective(f_x, GRB_MINIMIZE);
    }
};
```

## Multi-Objective Optimization

Weighted sum approach for multiple objectives.

```cpp
DECLARE_ENUM_WITH_COUNT(MultiObjVars, X)

class MultiObjectiveModel : public mini::ModelBuilder<MultiObjVars> {
private:
    int numVars = 10;
    std::vector<double> cost1, cost2;
    
public:
    void createVariables() override {
        vars.set(MultiObjVars::X,
            mini::VariableFactory::add(model, GRB_BINARY, 0, 1, "x", numVars));
    }

    void setWeightedSumObjective(double weight1, double weight2) {
        auto I = mini::dsl::indices(numVars);
        
        GRBLinExpr obj1 = mini::dsl::sum([&](int i) {
            return cost1[i] * vars.var(MultiObjVars::X, i);
        }, I);
        
        GRBLinExpr obj2 = mini::dsl::sum([&](int i) {
            return cost2[i] * vars.var(MultiObjVars::X, i);
        }, I);
        
        model.setObjective(weight1 * obj1 + weight2 * obj2, GRB_MINIMIZE);
    }
    
    // Generate Pareto frontier
    void generateParetoFrontier() {
        std::vector<std::pair<double, double>> paretoPoints;
        
        for (double w1 = 0.0; w1 <= 1.0; w1 += 0.1) {
            MultiObjectiveModel model;
            model.setWeightedSumObjective(w1, 1.0 - w1);
            auto result = model.solve(mini::RunOptions::quick());
            
            if (result.success) {
                // Extract both objective values
                double obj1 = /* compute obj1 from solution */;
                double obj2 = /* compute obj2 from solution */;
                paretoPoints.emplace_back(obj1, obj2);
            }
        }
    }
};
```

## Disjunctive Constraints

Model either-or conditions using Big-M.

```cpp
DECLARE_ENUM_WITH_COUNT(DisjunctiveVars, X, Y, MODE)

class DisjunctiveModel : public mini::ModelBuilder<DisjunctiveVars> {
private:
    int numTasks = 5;
    double bigM = 1000;

public:
    void createVariables() override {
        vars.set(DisjunctiveVars::X,
            mini::VariableFactory::add(model, GRB_CONTINUOUS, 0, 100, "x", numTasks));
        
        vars.set(DisjunctiveVars::Y,
            mini::VariableFactory::add(model, GRB_CONTINUOUS, 0, 100, "y", numTasks));
        
        vars.set(DisjunctiveVars::MODE,
            mini::VariableFactory::add(model, GRB_BINARY, 0, 1, "mode", numTasks));
    }

    void addConstraints() override {
        auto T = mini::dsl::indices(numTasks);
        
        // Either x[i] <= 10 OR y[i] >= 20
        FORALL([&](int i) {
            // Option 1: x[i] <= 10 when mode[i] = 1
            mini::constraint::conBigM_Le(model,
                vars.var(DisjunctiveVars::X, i), 10.0,
                vars.var(DisjunctiveVars::MODE, i), bigM);
            
            // Option 2: y[i] >= 20 when mode[i] = 0  
            mini::constraint::conBigM_Ge(model,
                vars.var(DisjunctiveVars::Y, i), 20.0,
                1 - vars.var(DisjunctiveVars::MODE, i), bigM);
        }, T);
        
        // Task non-overlap (classic disjunctive)
        FORALL([&](int i, int j) {
            if (i < j) {
                GRBVar precedes = model.addVar(0, 1, 0, GRB_BINARY, "precedes");
                
                // Either i precedes j or j precedes i
                mini::constraint::conBigM_Le(model,
                    vars.var(DisjunctiveVars::X, i) + 5,  // duration
                    vars.var(DisjunctiveVars::X, j),
                    precedes, bigM);
                
                mini::constraint::conBigM_Le(model,
                    vars.var(DisjunctiveVars::X, j) + 5,
                    vars.var(DisjunctiveVars::X, i),
                    1 - precedes, bigM);
            }
        }, T, T);
    }
};
```

## Performance Optimization

### 1. Efficient Constraint Building
```cpp
// GOOD: Batch constraints
GRBLinExpr total = mini::dsl::sum([&](int i, int j) {
    return cost(i,j) * x(i,j);
}, I, J);
model.addConstr(total <= budget);

// AVOID: Individual constraints in nested loops
forall(i, n) {
    forall(j, n) {
        model.addConstr(x(i,j) <= capacity); // O(n²) API calls
    }
}
```

### 2. Solver Parameter Tuning
```cpp
void configureModel() override {
    // MIP performance
    model.set(GRB_IntParam_MIPFocus, 1);        // Focus on good solutions
    model.set(GRB_IntParam_VarBranch, 2);       // Strong branching
    model.set(GRB_DoubleParam_Heuristics, 0.1); // Moderate heuristics
    
    // LP performance  
    model.set(GRB_IntParam_Method, 1);          // Dual simplex
    model.set(GRB_IntParam_Threads, 1);         // Single thread for LP
}
```

### 3. Warm Starts
```cpp
void provideWarmStart() {
    std::vector<double> initialSolution = heuristicSolution();
    
    FORALL([&](int i, int j) {
        vars.var(Vars::X, i, j).set(GRB_DoubleAttr_Start, initialSolution[i][j]);
    }, I, J);
}
```

## Advanced Configuration

### Memory Management for Large Models
```cpp
void configureLargeModel() {
    model.set(GRB_DoubleParam_NodefileStart, 0.5);  // Start node file early
    model.set(GRB_StringParam_NodefileDir, "/fast/scratch/");
    model.set(GRB_IntParam_Threads, 4);             // Limit threads for memory
}
```

### Solution Pool
```cpp
void configureSolutionPool() {
    model.set(GRB_IntParam_PoolSearchMode, 2);  // Find n best solutions
    model.set(GRB_IntParam_PoolSolutions, 10);  // Keep 10 solutions
    model.set(GRB_DoubleParam_PoolGap, 0.1);    // 10% gap for pool
}
```

## Key Performance Tips

1. **Use tight variable bounds** when known
2. **Exploit sparsity** - only create necessary constraints  
3. **Choose appropriate variable types** (avoid integers when continuous suffice)
4. **Use efficient constraint patterns** (summations over individual constraints)
5. **Configure solver parameters** for your problem type
6. **Provide initial solutions** when available

---

*For basic usage, see [Getting Started](GETTING_STARTED.md)*
*For practical examples, see [Examples](EXAMPLES.md)*