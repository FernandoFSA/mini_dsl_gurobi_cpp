#pragma once

namespace common::constraint {

    struct RunOptions
    {
        double timeLimitSec = 0;   // 0 = no limit
        double mipGap = 0;         // 0 = default
        int threads = 0;           // 0 = Gurobi default
        bool verbose = true;       // false = silence solver log
    };

} // namespace common::constraint
