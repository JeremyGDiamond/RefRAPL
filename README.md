# RefRAPL
A Reference Implementation of C code to read the values of the RAPL MSRs for running energy use on linux. Includes a custom kernel module for low overhead.

# refRAPL.c 
A pure c program that takes a file name and a program to run.

NOTE: Must be run as sudo to use the msr kernal moduel

## build
```make``` 

## Data foamat

### [test_name]_pkg.data, [test_name]_pp0.data, [test_name]_pp1.data, [test_name]_drm.data & [test_name]_tim.data 
running dumps of 100 uint64_t values in binary format, first 4 are energy values, last one is a ms timestamp

### [test_name]_pts.data 
4 timestamps over the execustion window. All 4 are uint64_t
```uint64_t t1,t2,t3,t4;```

# useModRefRAPL.c
Run the same way as refRAPL.c but uses the /proc/rapl_ref_dump interface for low overhead. Can be run with normal user privs but the module must already be loaded

## ex 
``` ./build/useModRefRapl "python testPrograms/badfib.py 30" $(date -I'seconds') ``` 

Will run a test and name the output files in iso 8601 format to the second

## build
```make```

## Data format
### [test_name].data 
A running append of the 2048 samples of the rapl buffer collected every 2 seconds in the kernel. Saved in raw binary format. The buffer is a ring and there should be up to 48 redundent entries per 2 second dump. the struct looks like

```
struct raplMeasurement {
    uint64_t ms_timestamp;
    uint8_t errorpkg, errorpp0, errorpp1, errordram;
    uint64_t pkg, pp0, pp1, dram;
};
```

### [test_name]_pts.data 

4 timestamps over the execustion window. All 4 are uint64_t
```uint64_t t1,t2,t3,t4;```

# testPrograms

badfib.py: a bad recursive fib python program to test cpu usage

# .data to .csv COOP


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