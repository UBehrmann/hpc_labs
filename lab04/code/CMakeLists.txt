cmake_minimum_required(VERSION 3.10)
project(simd_demo C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(simd_demo
    main.c
)

add_executable(simd_exercice
    src/exercice.c
)

target_compile_options(simd_demo PRIVATE
    -O3 -g -march=native
)

target_compile_options(simd_exercice PRIVATE
    -O3 -g -march=native
)

