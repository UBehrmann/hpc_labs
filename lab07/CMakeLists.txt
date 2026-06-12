cmake_minimum_required(VERSION 3.10)
project(lab07 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(OpenMP)

set(LAB01_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lab01)
set(LAB01_INCLUDES ${LAB01_DIR}/include)
set(LAB01_COMMON
    ${LAB01_DIR}/src/ecg_utils.c
    ${LAB01_DIR}/src/csv_reader.c
    ${LAB01_DIR}/src/json_writer.c
    ${LAB01_DIR}/src/ecg_processing.c
)

# --- k-mers ---
add_executable(k-mer src/main.c)
target_compile_options(k-mer PRIVATE -O2)

add_executable(k-mer-st src/main_st.c)
target_compile_options(k-mer-st PRIVATE -O2)

add_executable(k-mer-mt src/main_mp.c)
target_compile_options(k-mer-mt PRIVATE -O2)
if(OpenMP_C_FOUND)
    target_link_libraries(k-mer-mt PRIVATE OpenMP::OpenMP_C)
endif()

# --- Pan-Tompkins streaming (OpenMP) ---
add_executable(ecg-mt
    src/ecg_stream_mp.c
    src/ecg_stream.c
    ${LAB01_COMMON}
)
target_include_directories(ecg-mt PRIVATE include ${LAB01_INCLUDES})
target_compile_options(ecg-mt PRIVATE -O2)
target_link_libraries(ecg-mt PRIVATE m)
if(OpenMP_C_FOUND)
    target_link_libraries(ecg-mt PRIVATE OpenMP::OpenMP_C)
endif()
