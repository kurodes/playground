cmake_minimum_required(VERSION 3.16)
project(tlb_flush)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_compile_options(
        -O0
        -Wall
)

add_definitions(
        -DNDEBUG
)

add_executable(test_msync
        test_msync.cc)
target_link_libraries(test_msync
        pthread)

add_executable(test_fsync
        test_fsync.cc)
target_link_libraries(test_fsync
        pthread)
