#!/bin/bash
# build.sh - Build and run MiniDSL examples

set -e  # Exit on error

echo "=== MiniDSL Build Script ==="

# Create build directory
mkdir -p build
cd build

# Check for Gurobi
if [ -z "$GUROBI_HOME" ]; then
    echo "Warning: GUROBI_HOME environment variable not set."
    echo "Trying common Gurobi locations..."
    
    # Try to find Gurobi
    if [ -d "/opt/gurobi" ]; then
        export GUROBI_HOME="/opt/gurobi"
    elif [ -d "/usr/local/gurobi" ]; then
        export GUROBI_HOME="/usr/local/gurobi" 
    else
        echo "Please set GUROBI_HOME to your Gurobi installation directory"
        exit 1
    fi
fi

echo "Using Gurobi at: $GUROBI_HOME"

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "Building project..."
make -j$(nproc)

# Run examples
echo -e "\n=== Running Examples ==="

if [ -f "./examples/basic_example" ]; then
    echo -e "\n--- Basic Example (Knapsack) ---"
    ./examples/basic_example
fi

if [ -f "./examples/advanced_example" ]; then
    echo -e "\n--- Advanced Example (Facility Location) ---"
    ./examples/advanced_example
fi

echo -e "\n=== Build Complete ==="