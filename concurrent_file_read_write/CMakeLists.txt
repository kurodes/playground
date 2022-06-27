cmake_minimum_required(VERSION 3.16)
project(concurrent_read_write)

set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
fetchcontent_declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        86add13493e5c881d7e4ba77fb91c1f57752b3a4
        GIT_PROGRESS TRUE
)
fetchcontent_makeavailable(googletest)
enable_testing()
include(GoogleTest)
add_executable(ConcurrentRWTests
        ConcurrentRWTests.cpp)
target_link_libraries(ConcurrentRWTests
        gtest_main)
add_compile_options(
        -Wall
        -O2
        -g
)
gtest_discover_tests(ConcurrentRWTests)