cmake_minimum_required(VERSION 3.10)
project(roofline_demo C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(roofline_demo
    main.c
    src/roofline_demo.c
)

target_include_directories(roofline_demo PRIVATE include)

target_compile_options(roofline_demo PRIVATE
    -O0 -g -Wall
)

target_compile_definitions(roofline_demo PRIVATE
    LIKWID_PERFMON
)

target_link_libraries(roofline_demo PRIVATE likwid)
