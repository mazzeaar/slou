cmake_minimum_required(VERSION 3.15)  
project(slou VERSION 0.1 DESCRIPTION "A simple chess engin")

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
set(CMAKE_OSX_ARCHITECTURES "arm64")

include_directories(include)

# add json parser
find_package(nlohmann_json 3.10.5 REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(slou ${SOURCES})
target_link_libraries(slou PRIVATE nlohmann_json::nlohmann_json)

# binary output directory
set_target_properties(slou PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

# run the executable
add_custom_target(run
    COMMAND slou
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    COMMENT "Running the engine..."
    USES_TERMINAL
)

# perft test
add_custom_target(test
    COMMAND ../test.sh
    USES_TERMINAL
)