cmake_minimum_required(VERSION 3.10)
project(analyze LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(COMMON_PROFILE_FLAGS
    -O2
    -march=native
    -g
)

function(add_case target source)
    add_executable(${target} ${source})
    target_compile_options(${target} PRIVATE
        ${COMMON_PROFILE_FLAGS}
    )
endfunction()

add_case(case1         src/case1.c)
add_case(case2         src/case2.c)
add_case(case3         src/case3.c)
add_case(case4         src/case4.c)
add_case(main          src/main.c)
add_case(create_sample src/create-sample.c)

target_link_libraries(create_sample PRIVATE m)