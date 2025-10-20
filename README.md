# RefRAPL
A Reference Implementation of C code to read the values of the RAPL MSRs for running energy use on linux. Includes a custom kernel module for low overhead. Built on a nixos system, userspace should be portable to other linux distros.

# refRAPL.c 
A pure c program that takes a file name and a program to run.

NOTE: Must be run as sudo to use the msr kernal moduel

## ex 
``` sudo ./build/useModRefRapl "python testPrograms/badfib.py 30" a ``` 

Will run a test and name the output files starting with a

## build
```make``` 

## Data format

### [test_name].data  
A running append of the 100 samples of the rapl registers with a timestamp collected every milisecond. Saved in raw binary format. The struct looks like

```
struct raplMeasurement {
    u_int64_t ms_timestamp;
    u_int64_t pkg, pp0, pp1, dram;
};
```

### [test_name]_pts.data 
4 timestamps over the execustion window. All 4 are u_int64_t
```u_int64_t t1,t2,t3,t4;```

# useModRefRAPL.c
Run the same way as refRAPL.c but uses the /proc/rapl_ref_dump interface for low overhead. Can be run with normal user privs but the module must already be loaded

## ex 
``` ./build/useModRefRapl "python testPrograms/badfib.py 30" a ``` 

Will run a test and name the output files starting with a

## build
```make```

## Data format
### [test_name].data 
A running append of the 128 samples of the rapl buffer collected every 0.1 seconds in the kernel. Saved in raw binary format. The buffer is a ring and there should be up to 48 redundent entries per 2 second dump. The struct looks like

```
struct raplMeasurement {
    u_int64_t ms_timestamp;
    u_int64_t pkg, pp0, pp1, dram;
};
```

### [test_name]_pts.data 

4 timestamps over the execustion window. All 4 are u_int64_t
```u_int64_t t1,t2,t3,t4;```

# dataToCSV.c

Takes a testname and makes a testname_output.csv from testname.data and testname_pts.data

## ex

For a test in the data folder named a

``` ./build/dataToCsv data/a ```

NOTE: If the files were made with sudo (for example becuse you used sudo refRAPL.c) you may get and error unless you use sudo again.

## build

``` make```


# testPrograms

badfib.py: a bad recursive fib python program to test cpu usage


# kernMod/src/rapl_ref_mod.c
A custom out of tree kernel module that exposes 2 proc devices. Keeps 2048 samples in a ring buffer in kernel space

## /proc/rapl_ref
outputs a format string converted version of the current measurement buffer, good for debugging

## /proc/rapl_ref_dump
outputs a binary dump of the current measurement buffer

## build
Built on a nixos system using nix-build in the kenrMod directory. The shell.nix ensures you have the needed dependecies and provides a qemu setup for testing

```
cd kernMod
nix-build
```