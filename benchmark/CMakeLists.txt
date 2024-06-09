include(FetchContent)

FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG        v1.8.4
)
FetchContent_MakeAvailable(benchmark)

add_executable(spsc_queue_bench spsc_queue_bench.cpp)

target_link_libraries(spsc_queue_bench
        PRIVATE
        benchmark::benchmark
        mystl::lib)