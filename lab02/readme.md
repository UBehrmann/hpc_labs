# Lab 02

# likwid-topology

´´´bash
baer@baer-laptop:~/projects/hpc_labs/lab02$ likwid-topology
--------------------------------------------------------------------------------
CPU name:       AMD Ryzen 7 5800H with Radeon Graphics         
CPU type:       AMD K19 (Zen3) architecture
CPU stepping:   0
********************************************************************************
Hardware Thread Topology
********************************************************************************
Sockets:                1
CPU dies:               1
Cores per socket:       8
Threads per core:       2
--------------------------------------------------------------------------------
HWThread        Thread        Core        Die        Socket        Available
0               0             0           0          0             *                
1               1             0           0          0             *                
2               0             1           0          0             *                
3               1             1           0          0             *                
4               0             2           0          0             *                
5               1             2           0          0             *                
6               0             3           0          0             *                
7               1             3           0          0             *                
8               0             4           0          0             *                
9               1             4           0          0             *                
10              0             5           0          0             *                
11              1             5           0          0             *                
12              0             6           0          0             *                
13              1             6           0          0             *                
14              0             7           0          0             *                
15              1             7           0          0             *                
--------------------------------------------------------------------------------
Socket 0:               ( 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 )
--------------------------------------------------------------------------------
********************************************************************************
Cache Topology
********************************************************************************
Level:                  1
Size:                   32 kB
Cache groups:           ( 0 1 ) ( 2 3 ) ( 4 5 ) ( 6 7 ) ( 8 9 ) ( 10 11 ) ( 12 13 ) ( 14 15 )
--------------------------------------------------------------------------------
Level:                  2
Size:                   512 kB
Cache groups:           ( 0 1 ) ( 2 3 ) ( 4 5 ) ( 6 7 ) ( 8 9 ) ( 10 11 ) ( 12 13 ) ( 14 15 )
--------------------------------------------------------------------------------
Level:                  3
Size:                   16 MB
Cache groups:           ( 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 )
--------------------------------------------------------------------------------
********************************************************************************
NUMA Topology
********************************************************************************
NUMA domains:           1
--------------------------------------------------------------------------------
Domain:                 0
Processors:             ( 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 )
Distances:              10
Free memory:            15462.4 MB
Total memory:           27910.5 MB
--------------------------------------------------------------------------------
´´´

# likwid-bench

´´´bash
baer@baer-laptop:~/projects/hpc_labs/lab02$   likwid-bench -t peakflops -W N:30kB:1
Allocate: Process running on hwthread 0 (Domain N) - Vector length 3750/30000 Offset 0 Alignment 512
--------------------------------------------------------------------------------
LIKWID MICRO BENCHMARK
Test: peakflops
--------------------------------------------------------------------------------
Using 1 work groups
Using 1 threads
--------------------------------------------------------------------------------
Running without Marker API. Activate Marker API with -m on commandline.
--------------------------------------------------------------------------------
Group: 0 Thread 0 Global Thread 0 running on hwthread 0 - Vector length 3750 Offset 07864320000
MByte/s:                5356.17
Cycles per update:      4.763200
Cycles per cacheline:   38.105602
Loads per update:       1
Stores per update:      0
Load bytes per element: 8
Store bytes per elem.:  0
Instructions:           19660800032
UOPs:                   18677760000
--------------------------------------------------------------------------------
´´´

´´´bash
baer@baer-laptop:~/projects/hpc_labs/lab02$   likwid-bench -t load -W N:2GB:1
Allocate: Process running on hwthread 0 (Domain N) - Vector length 250000000/2000000000 Offset 0 Alignment 512
--------------------------------------------------------------------------------
LIKWID MICRO BENCHMARK
Test: load
--------------------------------------------------------------------------------
Using 1 work groups
Using 1 threads
--------------------------------------------------------------------------------
Running without Marker API. Activate Marker API with -m on commandline.
--------------------------------------------------------------------------------
Group: 0 Thread 0 Global Thread 0 running on hwthread 0 - Vector length 250000000 Offset 0
--------------------------------------------------------------------------------
Cycles:                 3926129056
CPU Clock:              3195000767
Cycle Clock:            3195000767
Time:                   1.228835e+00 sec
Iterations:             16
Iterations per thread:  16
Inner loop executions:  31250000
Size (Byte):            2000000000
Size per thread:        2000000000
Number of Flops:        0
MFlops/s:               0.00
Data volume (Byte):     32000000000
MByte/s:                26040.92
Cycles per update:      0.981532
Cycles per cacheline:   7.852258
Loads per update:       1
Stores per update:      0
Load bytes per element: 8
Store bytes per elem.:  0
Instructions:           5500000016
UOPs:                   5000000000
--------------------------------------------------------------------------------
´´´  15728640000
MFlops/s:               10712.35
Data volume (Byte):     7864320000
MByte/s:                5356.17
Cycles per update:      4.763200
Cycles per cacheline:   38.105602
Loads per update:       1
Stores per update:      0
Load bytes per element: 8
Store bytes per elem.:  0
Instructions:           19660800032
UOPs:                   18677760000
--------------------------------------------------------------------------------
´´´

´´´bash
baer@baer-laptop:~/projects/hpc_labs/lab02$   likwid-bench -t load -W N:2GB:1
Allocate: Process running on hwthread 0 (Domain N) - Vector length 250000000/2000000000 Offset 0 Alignment 512
--------------------------------------------------------------------------------
LIKWID MICRO BENCHMARK
Test: load
--------------------------------------------------------------------------------
Using 1 work groups
Using 1 threads
--------------------------------------------------------------------------------
Running without Marker API. Activate Marker API with -m on commandline.
--------------------------------------------------------------------------------
Group: 0 Thread 0 Global Thread 0 running on hwthread 0 - Vector length 250000000 Offset 0
--------------------------------------------------------------------------------
Cycles:                 3926129056
CPU Clock:              3195000767
Cycle Clock:            3195000767
Time:                   1.228835e+00 sec
Iterations:             16
Iterations per thread:  16
Inner loop executions:  31250000
Size (Byte):            2000000000
Size per thread:        2000000000
Number of Flops:        0
MFlops/s:               0.00
Data volume (Byte):     32000000000
MByte/s:                26040.92
Cycles per update:      0.981532
Cycles per cacheline:   7.852258
Loads per update:       1
Stores per update:      0
Load bytes per element: 8
Store bytes per elem.:  0
Instructions:           5500000016
UOPs:                   5000000000
--------------------------------------------------------------------------------
´´´

## Your Roofline Parameters

| Metric       | Value              | Source              |
| ------------ | ------------------ | ------------------- |
| maxperf      | 10,712.35 MFlops/s | peakflops benchmark |
| maxbandwidth | 26,040.92 MB/s     | load benchmark      |