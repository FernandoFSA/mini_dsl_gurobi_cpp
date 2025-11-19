# Examples for MiniDSL

# Basic knapsack example
add_executable(basic_example BasicExample.cpp)
target_link_libraries(basic_example ${GUROBI_LIBRARIES})

# Advanced facility location example  
add_executable(advanced_example AdvancedExample.cpp)
target_link_libraries(advanced_example ${GUROBI_LIBRARIES})

# Compiler optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(basic_example PRIVATE -O2 -Wall)
    target_compile_options(advanced_example PRIVATE -O2 -Wall)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(basic_example PRIVATE -g -DDEBUG_VARIABLE_NAMES=1)
        target_compile_options(advanced_example PRIVATE -g -DDEBUG_VARIABLE_NAMES=1)
    endif()
endif()

message(STATUS "Examples configured: basic_example, advanced_example")