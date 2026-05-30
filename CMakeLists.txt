cmake_minimum_required(VERSION 3.10)

project(AstraLog LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)
find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)

set(SEQUENTIAL_SOURCES
    src/main.cpp
    src/astralog.cpp
    src/astralog_data.cpp
    src/astralog_rules.cpp
    src/astralog_pipeline.cpp
    src/astralog_io.cpp
    src/astralog_input.cpp
)

add_executable(astralog-seq ${SEQUENTIAL_SOURCES})
target_include_directories(astralog-seq PUBLIC include)
target_link_libraries(astralog-seq PRIVATE nlohmann_json::nlohmann_json)
