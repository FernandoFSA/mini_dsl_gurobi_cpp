#include "../ModelBuilderGeneric.h"
#include "../VariableFactory.h"
#include "../Comprehension.h"
#include "../SumOver.h"
#include "../ConstraintHelpers.h"
#include "../dsl_macros.h"
#include <iostream>

/**
 * Facility Location Problem
 * Choose facilities to open and assign customers to minimize total cost
 */
enum class FacilityVars { Open, Assign, MAX };

class FacilityLocationModel : public mini::ModelBuilderGeneric<FacilityVars,
                                    static_cast<size_t>(FacilityVars::MAX)> {
private:
    int n_facilities;
    int n_customers;
    std::vector<double> fixed_costs;
    std::vector<std::vector<double>> assignment_costs;
    
public:
    FacilityLocationModel(int n_fac, int n_cust, 
                         const std::vector<double>& f_costs,
                         const std::vector<std::vector<double>>& a_costs)
        : n_facilities(n_fac), n_customers(n_cust), 
          fixed_costs(f_costs), assignment_costs(a_costs) {}
    
    void createVariables() override {
        // Binary variables: open[i] = 1 if facility i is open
        getVars().set(FacilityVars::Open,
            VariableFactory::create(getModel(), GRB_BINARY, 0, 1, "open", n_facilities));
            
        // Binary variables: assign[i][j] = 1 if customer j assigned to facility i
        getVars().set(FacilityVars::Assign,
            VariableFactory::create(getModel(), GRB_BINARY, 0, 1, "assign", 
                                   n_facilities, n_customers));
    }
    
    void addConstraints() override {
        auto F = mini::dsl::indexSet(n_facilities);
        auto C = mini::dsl::indexSet(n_customers);
        
        // Each customer assigned to exactly one facility
        for (int j : C) {
            GRBLinExpr total_assignment = 0;
            for (int i : F) {
                total_assignment += getVars().var(FacilityVars::Assign, i, j);
            }
            mini::constraint::addEq(getModel(), total_assignment, 1, 
                                  mini::constraint::cnameND("assign_one", j));
        }
        
        // If customer assigned to facility, facility must be open
        for (int i : F) {
            for (int j : C) {
                auto& assign = getVars().var(FacilityVars::Assign, i, j);
                auto& open = getVars().var(FacilityVars::Open, i);
                mini::constraint::addLe(getModel(), assign, open, 
                                      mini::constraint::cnameND("facility_open", i, j));
            }
        }
        
        // Demonstrate comprehension syntax
        using namespace mini::dsl;
        auto I = indexSet(n_facilities);
        auto J = indexSet(n_customers);
        
        // Alternative formulation using comprehensions
        // comp((j) in J, ...) creates constraints for each customer
        auto constraints = comp(Index1D{J}, [&](int j) {
            GRBLinExpr sum_assign = 0;
            for (int i : I) sum_assign += getVars().var(FacilityVars::Assign, i, j);
            return std::make_pair(sum_assign, 1.0); // (lhs, rhs)
        });
        
        for (size_t j = 0; j < constraints.size(); ++j) {
            mini::constraint::addEq(getModel(), constraints[j].first, constraints[j].second,
                                  mini::constraint::cnameND("comp_assign", (int)j));
        }
    }
    
    void setObjective() override {
        auto F = mini::dsl::indexSet(n_facilities);
        auto C = mini::dsl::indexSet(n_customers);
        
        // Total cost = sum(fixed_costs[i] * open[i]) + sum(assignment_costs[i][j] * assign[i][j])
        GRBLinExpr total_cost = 0;
        
        // Fixed costs
        for (int i : F) {
            total_cost += fixed_costs[i] * getVars().var(FacilityVars::Open, i);
        }
        
        // Assignment costs using sumOver
        total_cost += mini::dsl::sumOver(F, C, [&](int i, int j) {
            return assignment_costs[i][j] * getVars().var(FacilityVars::Assign, i, j);
        });
        
        getModel().setObjective(total_cost, GRB_MINIMIZE);
    }
};

int main() {
    // Problem data
    int n_facilities = 3;
    int n_customers = 5;
    
    std::vector<double> fixed_costs = {100.0, 150.0, 120.0};
    std::vector<std::vector<double>> assignment_costs = {
        {10.0, 15.0, 20.0, 25.0, 30.0},
        {20.0, 25.0, 15.0, 30.0, 10.0},
        {15.0, 20.0, 25.0, 10.0, 30.0}
    };
    
    // Build and solve model
    FacilityLocationModel model(n_facilities, n_customers, fixed_costs, assignment_costs);
    
    mini::RunOptions opts;
    opts.timeLimitSec = 30.0;
    opts.mipGap = 0.01;
    opts.threads = 4;
    opts.verbose = true;
    
    auto result = model.solve(opts);
    
    // Output results
    std::cout << "\n=== Facility Location Results ===" << std::endl;
    std::cout << "Success: " << (result.success ? "Yes" : "No") << std::endl;
    std::cout << "Status: " << result.status << std::endl;
    std::cout << "Objective: " << result.objective << std::endl;
    std::cout << "Runtime: " << result.runtimeSec << "s" << std::endl;
    
    if (result.success) {
        std::cout << "\nOpen facilities: ";
        for (int i = 0; i < n_facilities; ++i) {
            try {
                double val = model.getVars().var(FacilityVars::Open, i).get(GRB_DoubleAttr_X);
                if (val > 0.5) {
                    std::cout << "F" << i << " ";
                }
            } catch (...) {}
        }
        
        std::cout << "\n\nAssignments:" << std::endl;
        for (int j = 0; j < n_customers; ++j) {
            for (int i = 0; i < n_facilities; ++i) {
                try {
                    double val = model.getVars().var(FacilityVars::Assign, i, j).get(GRB_DoubleAttr_X);
                    if (val > 0.5) {
                        std::cout << "Customer " << j << " -> Facility " << i << std::endl;
                    }
                } catch (...) {}
            }
        }
    }
    
    return 0;
}