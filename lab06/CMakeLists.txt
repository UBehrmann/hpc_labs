cmake_minimum_required(VERSION 3.10)
project(lab06_energy C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(powercap)

add_executable(simd_bench src/simd_bench.c)
target_compile_options(simd_bench PRIVATE -O3 -march=native)

add_library(perf_manager STATIC src/perf_manager.c)
target_include_directories(perf_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LAB01_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lab01)
set(LAB01_COMMON
    ${LAB01_DIR}/src/ecg_utils.c
    ${LAB01_DIR}/src/csv_reader.c
    ${LAB01_DIR}/src/json_writer.c
)
set(LAB01_SOURCES
    ${LAB01_DIR}/src/ecg_processing.c
    ${LAB01_COMMON}
)

add_executable(ecg_energy
    src/ecg_energy.c
    ${LAB01_SOURCES}
)
target_include_directories(ecg_energy PRIVATE
    ${LAB01_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(ecg_energy PRIVATE perf_manager m matio)

add_executable(ecg_energy_avant
    src/ecg_energy.c
    baseline/src/ecg_processing.c
    ${LAB01_COMMON}
)
target_include_directories(ecg_energy_avant PRIVATE
    baseline/include
    ${LAB01_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(ecg_energy_avant PRIVATE perf_manager m matio)

add_executable(ecg_repeat_avant
    src/ecg_repeat.c
    baseline/src/ecg_processing.c
    ${LAB01_DIR}/src/csv_reader.c
    ${LAB01_DIR}/src/ecg_utils.c
)
target_include_directories(ecg_repeat_avant PRIVATE
    baseline/include
    ${LAB01_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(ecg_repeat_avant PRIVATE perf_manager m)

add_executable(ecg_repeat_apres
    src/ecg_repeat.c
    ${LAB01_DIR}/src/ecg_processing.c
    ${LAB01_DIR}/src/csv_reader.c
    ${LAB01_DIR}/src/ecg_utils.c
)
target_include_directories(ecg_repeat_apres PRIVATE
    ${LAB01_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(ecg_repeat_apres PRIVATE perf_manager m)

add_executable(ecg_powercap
    src/ecg_powercap.c
    ${LAB01_SOURCES}
)
target_include_directories(ecg_powercap PRIVATE
    ${LAB01_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/powercap/inc
)
target_link_libraries(ecg_powercap PRIVATE powercap m matio)
