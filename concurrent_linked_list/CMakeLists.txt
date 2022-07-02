cmake_minimum_required(VERSION 3.16)
project(list)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_compile_options(
        -Wall
        -O2
        -g
)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")

add_executable(Main
        main.cpp
        NaiveList.h
        CoarseLockList.h
        LockCouplingList.h
        OptimisticList.h
        List.h)
target_link_libraries(Main pthread)
target_compile_definitions(Main PUBLIC -DOPTIMISTICLIST)

#google test
include(FetchContent)
set(fetchcontent_quiet false)
fetchcontent_declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG 86add13493e5c881d7e4ba77fb91c1f57752b3a4
)
fetchcontent_makeavailable(googletest)
enable_testing()
include(GoogleTest)
foreach (x NaiveList CoarseLockList LockSingleList LockCouplingList OptimisticList OptimisticLazyList LockFreeList)
    add_executable(${x}Tests ListTests.cpp ${x}.h List.h)
    target_link_libraries(${x}Tests gtest_main)
    string(TOUPPER ${x} X)
#    message(STATUS "x=${x},X=${X}")
    target_compile_definitions(${x}Tests PUBLIC -D${X})
    gtest_discover_tests(${x}Tests)
endforeach ()
