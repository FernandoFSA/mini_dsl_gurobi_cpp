cmake_minimum_required(VERSION 3.15)
project(MiniDSL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Gurobi
if(NOT GUROBI_HOME)
    set(GUROBI_HOME $ENV{GUROBI_HOME})
endif()

if(GUROBI_HOME)
    message(STATUS "GUROBI_HOME: ${GUROBI_HOME}")
    set(GUROBI_INCLUDE_DIRS "${GUROBI_HOME}/include")
    
    # Find library based on platform
    if(UNIX AND NOT APPLE)
        set(GUROBI_LIBRARIES 
            "${GUROBI_HOME}/lib/libgurobi_g++5.2.a"
            "${GUROBI_HOME}/lib/libgurobi_c++.a"
            "${GUROBI_HOME}/lib/libgurobi100.so")
    elseif(APPLE)
        set(GUROBI_LIBRARIES 
            "${GUROBI_HOME}/lib/libgurobi_g++5.2.a"
            "${GUROBI_HOME}/lib/libgurobi_c++.a"
            "${GUROBI_HOME}/lib/libgurobi100.dylib")
    elseif(WIN32)
        set(GUROBI_LIBRARIES 
            "${GUROBI_HOME}/lib/gurobi_c++md.lib"
            "${GUROBI_HOME}/lib/gurobi100.lib")
    endif()
    
    include_directories(${GUROBI_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Gurobi not found. Please set GUROBI_HOME environment variable.")
endif()

# Add examples subdirectory
add_subdirectory(examples)

# Optional: Create a library if you want to use MiniDSL as a lib
# add_library(minidsl INTERFACE)
# target_include_directories(minidsl INTERFACE .)
# target_link_libraries(minidsl INTERFACE ${GUROBI_LIBRARIES})

message(STATUS "MiniDSL configured successfully")
message(STATUS "Build examples with: make")