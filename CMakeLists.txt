cmake_minimum_required(VERSION 3.25)
project(spsc_queue)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(mystl INTERFACE)
add_library(mystl::lib ALIAS mystl)
target_include_directories(mystl
        INTERFACE ${PROJECT_SOURCE_DIR}/include
)

enable_testing()
add_subdirectory(test)
add_subdirectory(benchmark)



