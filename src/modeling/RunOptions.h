#pragma once
/*
RunOptions.h
Solver configuration options for fine-grained control.

Features:
- Common solver parameters in one struct
- Sensible defaults
- Easy configuration for different scenarios

Examples:
  // Quick solve with time limit
  RunOptions opts{300.0};  // 5 minute limit

  // High-precision solve
  RunOptions opts{3600.0, 0.0001, 8, false};  // 1 hour, tight gap, 8 threads, quiet

  // Quick feasibility check
  RunOptions opts{60.0, 0.1, 1, false};  // 1 minute, 10% gap, single thread
*/

namespace mini {

    /// Solver configuration parameters
    struct RunOptions {
        double timeLimitSec = 0;      ///< Maximum solve time (0 = no limit)
        double mipGap = 0;           ///< Relative optimality gap (0 = solver default)
        int threads = 0;             ///< Number of threads (0 = solver default)
        bool verbose = true;         ///< Enable solver output
        int solutionLimit = 0;       ///< Maximum solutions to find (0 = no limit)
        double nodeLimit = 0;        ///< Maximum nodes to explore (0 = no limit)
        int presolve = -1;           ///< Presolve level (-1 = solver default)
        int method = -1;             ///< Solution method (-1 = automatic)

        RunOptions() = default;

        /// Convenience constructor for common parameters
        RunOptions(double timeLimit, double gap = 0, int thr = 0, bool verb = true)
            : timeLimitSec(timeLimit), mipGap(gap), threads(thr), verbose(verb) {
        }

        /// Create options for quick feasibility check
        static RunOptions quick() {
            return RunOptions(60.0, 0.1, 1, false);  // 1 minute, 10% gap
        }

        /// Create options for high-precision solve  
        static RunOptions precise() {
            return RunOptions(3600.0, 1e-6, 0, true);  // 1 hour, tight gap
        }

        /// Create options for performance testing
        static RunOptions performance() {
            RunOptions opts(0, 0, 0, false);  // No limits, quiet
            opts.presolve = 1;  // Aggressive presolve
            return opts;
        }
    };

} // namespace mini