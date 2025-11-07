cmake_minimum_required(VERSION 3.16)

project(traker VERSION 1.0)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(EVENT__DISABLE_TESTS ON CACHE BOOL "Disable libevent tests")
set(EVENT__DISABLE_BENCHMARK ON CACHE BOOL "Disable libevent benchmarks")
set(EVENT__DISABLE_REGRESS ON CACHE BOOL "Disable libevent regress tests")

add_subdirectory(external)
add_subdirectory(src)



