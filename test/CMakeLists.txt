include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(spsc_queue_test spsc_queue_test.cpp)

target_compile_options(spsc_queue_test PRIVATE
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
        -g
)

target_link_libraries(spsc_queue_test
        PRIVATE
        GTest::gtest_main
        mystl::lib)

target_link_options(spsc_queue_test PRIVATE
        -fsanitize=address
        -fsanitize=undefined
)