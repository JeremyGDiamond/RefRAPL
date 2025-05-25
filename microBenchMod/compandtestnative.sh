#!/usr/bin/env bash

nix-build

sudo insmod result/lib/modules/6.6.80/extra/micro_bench.ko

# ls /proc/micro_bench/

echo "name                       ,overhead(ns * 100000_Calls),ts0(ns),ts1(ns)"
cat /proc/micro_bench/cpuid
cat /proc/micro_bench/mov
cat /proc/micro_bench/nop
cat /proc/micro_bench/rdmsr*
../build/micro_syscall_overhead

sudo rmmod micro_bench