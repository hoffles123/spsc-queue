cmake_minimum_required(VERSION 3.28)
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

if (CMAKE_BUILD_TYPE MATCHES Debug)
    # Enable AddressSanitizer
    set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer")

    # Enable MemorySanitizer
    set(SANITIZER_FLAGS "-fsanitize=memory -fsanitize-memory-track-origins -fno-omit-frame-pointer")

    # Enable ThreadSanitizer
    set(SANITIZER_FLAGS "-fsanitize=thread")

    # Enable UndefinedBehaviorSanitizer
    set(SANITIZER_FLAGS "-fsanitize=undefined")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()

