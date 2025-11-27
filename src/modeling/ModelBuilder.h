#pragma once
/*
ModelBuilder.h
Template-based model building framework with solve orchestration.

Features:
- Structured model building (variables, constraints, objective)
- Automatic solve result handling
- Configurable solver options
- Error handling and status reporting

Examples:
  enum class Vars { X, Y, MAKESPAN, COUNT };
  // DECLARE_ENUM_WITH_COUNT(Vars, X, Y, MAKESPAN) // alternative macro

  class MyModel : public ModelBuilder<Vars> {
  protected:
      void createVariables() override {
          vars.set(Vars::X, VariableFactory::add(model, GRB_BINARY, 0, 1, "X", 10, 20));
          vars.set(Vars::Y, VariableFactory::add(model, GRB_CONTINUOUS, 0, 100, "Y", 5));
      }

      void addConstraints() override {
          auto I = indices(10), J = indices(20);
          forall2(i, 10, j, 20) {
              addLe(model, vars.var(Vars::X, i, j), vars.var(Vars::Y, i), "link_constraint");
          }
      }

      void setObjective() override {
          GRBLinExpr obj = sum(indices(10), [&](int i) { return vars.var(Vars::Y, i); });
          model.setObjective(obj, GRB_MINIMIZE);
      }
  };

  // Usage
  MyModel model;
  auto result = model.solve({.timeLimitSec = 300, .mipGap = 0.01});
  if (result.success) {
      std::cout << "Objective: " << result.objective << "\n";
  }
*/

#include <chrono>
#include <format>
#include "gurobi_c++.h"
#include "../core/VariableTable.h"

namespace mini {

    /// Solver configuration options
    struct RunOptions {
        double timeLimitSec = 0;      ///< 0 = no limit
        double mipGap = 0;           ///< 0 = solver default
        int threads = 0;             ///< 0 = solver default  
        bool verbose = true;         ///< Enable solver output
        int solutionLimit = 0;       ///< 0 = no limit
        double nodeLimit = 0;        ///< 0 = no limit

        RunOptions() = default;

        /// Convenience constructor
        RunOptions(double timeLimit, double gap = 0, int thr = 0, bool verb = true)
            : timeLimitSec(timeLimit), mipGap(gap), threads(thr), verbose(verb) {
        }
    };

    /// Model building framework with solve orchestration
    template<typename EnumT, size_t MAX = static_cast<size_t>(EnumT::COUNT)>
    class ModelBuilder {
    protected:
        GRBEnv env;                          ///< Gurobi environment
        GRBModel model;                      ///< Gurobi model
        VariableTable<EnumT, MAX> vars;      ///< Variable storage

    public:
        ModelBuilder() : env(), model(env) {
            // Start with output disabled, enable in solve() if requested
            model.set(GRB_IntParam_OutputFlag, 0);
        }

        virtual ~ModelBuilder() = default;

        // ============================================================================
        // MODEL BUILDING INTERFACE (Override these)
        // ============================================================================

        /// Create all variables - must be implemented by derived class
        virtual void createVariables() = 0;

        /// Add all constraints - must be implemented by derived class  
        virtual void addConstraints() = 0;

        /// Set the objective function - must be implemented by derived class
        virtual void setObjective() = 0;

        /// Optional model configuration (presolve, parameters, etc.)
        virtual void configureModel() {}

        // ============================================================================
        // SOLVE ORCHESTRATION
        // ============================================================================

        /// Solve result information
        struct SolveResult {
            bool success = false;            ///< Solve completed without error
            int status = -1;                 ///< Gurobi status code
            double objective = 0.0;          ///< Best objective value found
            double runtimeSec = 0.0;         ///< Total solve time
            int nodeCount = 0;               ///< Nodes explored
            double gap = 0.0;                ///< Final optimality gap
            GRBModel* model = nullptr;       ///< Pointer to solved model
            std::string errorMsg;            ///< Error description if failed

            /// Check if solution is optimal
            bool isOptimal() const { return status == GRB_OPTIMAL; }

            /// Check if solution is feasible (optimal or suboptimal)
            bool hasSolution() const {
                return status == GRB_OPTIMAL || status == GRB_SUBOPTIMAL ||
                    status == GRB_TIME_LIMIT || status == GRB_NODE_LIMIT ||
                    status == GRB_SOLUTION_LIMIT;
            }
        };

        /// Build the complete model
        GRBModel& buildModel() {
            configureModel();
            model.update();
            return model;
        }

        /// Solve the model with given options
        SolveResult solve(const RunOptions& opts = {}) {
            SolveResult result;
            result.model = &model;
            auto startTime = std::chrono::high_resolution_clock::now();

            try {
                // Build the model
                createVariables();
                addConstraints();
                setObjective();
                buildModel();

                // Apply solver options
                if (opts.timeLimitSec > 0)
                    model.set(GRB_DoubleParam_TimeLimit, opts.timeLimitSec);
                if (opts.mipGap > 0)
                    model.set(GRB_DoubleParam_MIPGap, opts.mipGap);
                if (opts.threads > 0)
                    model.set(GRB_IntParam_Threads, opts.threads);
                if (opts.solutionLimit > 0)
                    model.set(GRB_IntParam_SolutionLimit, opts.solutionLimit);
                if (opts.nodeLimit > 0)
                    model.set(GRB_DoubleParam_NodeLimit, opts.nodeLimit);

                model.set(GRB_IntParam_OutputFlag, opts.verbose ? 1 : 0);

                // Solve
                model.optimize();

                // Capture results
                auto endTime = std::chrono::high_resolution_clock::now();
                result.runtimeSec = std::chrono::duration<double>(endTime - startTime).count();
                result.status = model.get(GRB_IntAttr_Status);
                result.nodeCount = static_cast<int>(model.get(GRB_DoubleAttr_NodeCount));

                if (result.hasSolution()) {
                    try {
                        result.objective = model.get(GRB_DoubleAttr_ObjVal);
                        result.gap = model.get(GRB_DoubleAttr_MIPGap);
                    }
                    catch (...) {
                        // Objective value not available
                    }
                }

                result.success = true;
            }
            catch (GRBException& e) {
                result.success = false;
                result.errorMsg = std::format("Gurobi Error {}: {}", e.getErrorCode(), e.getMessage());
            }
            catch (std::exception& e) {
                result.success = false;
                result.errorMsg = std::string("Exception: ") + e.what();
            }
            catch (...) {
                result.success = false;
                result.errorMsg = "Unknown exception during solve()";
            }

            return result;
        }

        // ============================================================================
        // ACCESSORS
        // ============================================================================

        GRBModel& getModel() { return model; }
        const GRBModel& getModel() const { return model; }

        VariableTable<EnumT, MAX>& getVars() { return vars; }
        const VariableTable<EnumT, MAX>& getVars() const { return vars; }

        GRBEnv& getEnv() { return env; }
        const GRBEnv& getEnv() const { return env; }

        // ============================================================================
        // MODEL ANALYSIS
        // ============================================================================

        // Helper: convert Gurobi status code to a human-readable string
        static const char* statusToString(int status) {
            switch (status) {
            case GRB_LOADED:        return "LOADED";
            case GRB_OPTIMAL:       return "OPTIMAL";
            case GRB_INFEASIBLE:    return "INFEASIBLE";
            case GRB_INF_OR_UNBD:   return "INF_OR_UNBD";
            case GRB_UNBOUNDED:     return "UNBOUNDED";
            case GRB_CUTOFF:        return "CUTOFF";
            case GRB_ITERATION_LIMIT:return "ITERATION_LIMIT";
            case GRB_NODE_LIMIT:    return "NODE_LIMIT";
            case GRB_TIME_LIMIT:    return "TIME_LIMIT";
            case GRB_SOLUTION_LIMIT:return "SOLUTION_LIMIT";
            case GRB_INTERRUPTED:   return "INTERRUPTED";
            case GRB_NUMERIC:       return "NUMERIC";
            case GRB_SUBOPTIMAL:    return "SUBOPTIMAL";
            default:                return "UNKNOWN";
            }
        }

        // Print model statistics
        void printStats(std::ostream& os = std::cout) const {
            int status = model.get(GRB_IntAttr_Status);
            os << "Model Summary:\n"
                << "  Variables: " << model.get(GRB_IntAttr_NumVars) << "\n"
                << "  Constraints: " << model.get(GRB_IntAttr_NumConstrs) << "\n"
                << "  Non-zeros: " << model.get(GRB_IntAttr_NumNZs) << "\n"
                << "  Status: " << status << " (" << statusToString(status) << ")\n";
        }

        // Write model to file
        void writeModel(const std::string& filename) {
            try {
                model.write(filename);
            }
            catch (const std::exception& e) {
                throw std::runtime_error(std::format("Failed to write model to {}: {}", filename, e.what()));
            }
        }
    };

} // namespace mini