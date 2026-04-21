# Lab04

## Build

´´´bash
mkdir build
cd build
cmake ..
make
´´´

## Execution

´´´bash
./simd_demo <nbr_points> <nbr_iterations>
´´´

## Baseline performance measurement

´´´bash
perf record -g ./simd_demo 10000 10000

perf report

perf annotate -i perf.data --stdio main > main_asm.txt
´´´

## Objdump

´´´bash
objdump -d simd_demo > simd_demo.asm
´´´

## Optimization with compiler

´´´bash


# Temps d'exécution

| Optimization         | Execution Time |
| -------------------- | -------------- |
| **O0**               | 252ms          |
| **O2**               | 40ms           |
| **O3**               | 37ms           |
| **O3 -march=native** | 27ms           |


