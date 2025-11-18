#pragma once
/*
ModelBuilderGeneric.h
Templated class that orchestrates building the model:
 - createVariables()
 - addConstraints()
 - setObjective()
 - configureModel() optional
 - solve(opts) convenience (build + apply RunOptions + optimize + return result)

Usage:
  class MyModel : public ModelBuilderGeneric<MyFamilyEnum, MAX> { ... };
  MyModel m(inst); auto res = m.solve(opts);
*/

#include <chrono>
#include <format>
#include "gurobi_c++.h"
#include "VariableTable.h"

namespace mini {

    struct RunOptions {
        double timeLimitSec = 0;
        double mipGap = 0;
        int threads = 0;
        bool verbose = true;
    };

    template<typename EnumT, size_t MAX>
    class ModelBuilderGeneric {
    protected:
        GRBEnv env;
        GRBModel model;
        VariableTable<EnumT, MAX> vars;

    public:
        ModelBuilderGeneric() : env(), model(env) { model.set(GRB_IntParam_OutputFlag, 0); }
        virtual ~ModelBuilderGeneric() = default;

        virtual void createVariables() = 0;
        virtual void addConstraints() = 0;
        virtual void setObjective() = 0;
        virtual void configureModel() {}

        GRBModel& buildModel() { configureModel(); model.update(); return model; }

        struct SolveResult {
            bool success = false;
            int status = -1;
            double objective = 0.0;
            double runtimeSec = 0.0;
            GRBModel* model = nullptr;
            std::string errorMsg;
        };

        SolveResult solve(const RunOptions& opts = {}) {
            SolveResult res; res.model = &model;
            auto t0 = std::chrono::high_resolution_clock::now();
            try {
                createVariables();
                addConstraints();
                setObjective();
                buildModel();

                if (opts.timeLimitSec > 0) model.set(GRB_DoubleParam_TimeLimit, opts.timeLimitSec);
                if (opts.mipGap > 0) model.set(GRB_DoubleParam_MIPGap, opts.mipGap);
                if (opts.threads > 0) model.set(GRB_IntParam_Threads, opts.threads);
                if (!opts.verbose) model.set(GRB_IntParam_OutputFlag, 0);

                model.optimize();

                auto t1 = std::chrono::high_resolution_clock::now();
                res.runtimeSec = std::chrono::duration<double>(t1 - t0).count();
                res.status = model.get(GRB_IntAttr_Status);
                if (res.status == GRB_OPTIMAL || res.status == GRB_SUBOPTIMAL ||
                    res.status == GRB_TIME_LIMIT || res.status == GRB_ITERATION_LIMIT ||
                    res.status == GRB_SOLUTION_LIMIT)
                {
                    try { res.objective = model.get(GRB_DoubleAttr_ObjVal); }
                    catch (...) { res.objective = 0.0; }
                }
                res.success = true;
            }
            catch (GRBException& e) {
                res.success = false; res.errorMsg = std::format("Gurobi Error {}: {}", e.getErrorCode(), e.getMessage());
            }
            catch (std::exception& e) {
                res.success = false; res.errorMsg = std::string("Exception: ") + e.what();
            }
            catch (...) {
                res.success = false; res.errorMsg = "Unknown exception during solve()";
            }
            return res;
        }

        GRBModel& getModel() { return model; }
        VariableTable<EnumT, MAX>& getVars() { return vars; }
    };

} // namespace mini
