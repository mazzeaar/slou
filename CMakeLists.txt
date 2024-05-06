cmake_minimum_required(VERSION 3.15)  
project(Engine VERSION 0.1 DESCRIPTION "A simple chess engine")

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra -march=native")

include_directories(include)

include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# add json parser
find_package(nlohmann_json 3.10.5 REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(Engine ${SOURCES})
target_link_libraries(Engine PRIVATE nlohmann_json::nlohmann_json)

# binary output directory
set_target_properties(Engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

# run the executable
add_custom_target(run
    COMMAND Engine
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    COMMENT "Running the chess engine"
    USES_TERMINAL
)

# perft test
add_custom_target(perft
    COMMAND Engine --perft
    COMMENT "Running perft tests"
    USES_TERMINAL
)
