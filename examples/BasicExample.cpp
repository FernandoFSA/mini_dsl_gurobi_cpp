#include "../ModelBuilderGeneric.h"
#include "../VariableFactory.h"
#include "../SumOver.h"
#include "../ConstraintHelpers.h"
#include "../ForallN.h"
#include <iostream>

/**
 * Basic Knapsack Problem Example
 * Maximize value without exceeding weight capacity
 */
enum class KnapsackVars { X, MAX };

class KnapsackModel : public mini::ModelBuilderGeneric<KnapsackVars, 
                                  static_cast<size_t>(KnapsackVars::MAX)> {
private:
    int n_items;
    std::vector<double> values;
    std::vector<double> weights;
    double capacity;
    
public:
    KnapsackModel(int n, const std::vector<double>& v, 
                  const std::vector<double>& w, double cap) 
        : n_items(n), values(v), weights(w), capacity(cap) {}
    
    void createVariables() override {
        // Binary variables: x_i = 1 if item i is selected
        getVars().set(KnapsackVars::X,
            VariableFactory::create(getModel(), GRB_BINARY, 0, 1, "x", n_items));
    }
    
    void addConstraints() override {
        // Weight capacity constraint
        GRBLinExpr total_weight = 0;
        for (int i = 0; i < n_items; ++i) {
            total_weight += weights[i] * getVars().var(KnapsackVars::X, i);
        }
        mini::constraint::addLe(getModel(), total_weight, capacity, "capacity");
    }
    
    void setObjective() override {
        // Maximize total value
        GRBLinExpr total_value = 0;
        for (int i = 0; i < n_items; ++i) {
            total_value += values[i] * getVars().var(KnapsackVars::X, i);
        }
        getModel().setObjective(total_value, GRB_MAXIMIZE);
    }
};

int main() {
    // Problem data
    int n_items = 5;
    std::vector<double> values = {10.0, 20.0, 15.0, 25.0, 30.0};
    std::vector<double> weights = {1.0, 3.0, 2.0, 4.0, 5.0};
    double capacity = 8.0;
    
    // Build and solve
    KnapsackModel model(n_items, values, weights, capacity);
    mini::RunOptions opts;
    opts.timeLimitSec = 10.0;
    opts.verbose = true;
    
    auto result = model.solve(opts);
    
    // Output results
    std::cout << "=== Knapsack Problem Results ===" << std::endl;
    std::cout << "Status: " << (result.success ? "Success" : "Failed") << std::endl;
    if (result.success) {
        std::cout << "Objective value: " << result.objective << std::endl;
        std::cout << "Runtime: " << result.runtimeSec << " seconds" << std::endl;
        
        // Print selected items
        std::cout << "Selected items: ";
        for (int i = 0; i < n_items; ++i) {
            try {
                double val = model.getVars().var(KnapsackVars::X, i).get(GRB_DoubleAttr_X);
                if (val > 0.5) {
                    std::cout << i << " ";
                }
            } catch (...) {
                // Variable might not be available if solve failed
            }
        }
        std::cout << std::endl;
    } else {
        std::cout << "Error: " << result.errorMsg << std::endl;
    }
    
    return 0;
}