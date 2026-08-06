#define _GNU_SOURCE

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define PROC_PATH "/proc/micro_bench/sys_call_overhead_proc"
#define SYS_PATH "/sys/kernel/syscall_bench/sys_call_overhead"
#define REPS 100000

uint64_t timespec_to_ns(struct timespec ts) {
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// Read two kernel timestamps (u64 each)
int main() {
    int fd;
    uint64_t kernel_ts[2];
    struct timespec user_before, user_after;
    uint64_t udelta = 0;
    uint64_t kdelta = 0;
    uint64_t per_call_overhead = 0;
    uint64_t total_proc_overhead = 0;
    char buf[128];
    uint64_t total_sys_overhead = 0;

    for (size_t i = 0; i < REPS ; i++){

        fd = open(PROC_PATH, O_RDONLY);
        if (fd < 0) {
            perror("open");
            return 1;
        }
    
        // Get timestamp before syscall
        clock_gettime(CLOCK_MONOTONIC_RAW, &user_before);

        // Read the kernel-generated timestamps
        ssize_t res = read(fd, kernel_ts, sizeof(kernel_ts));

        if (res < 0) {
            perror("read");
            close(fd);
            return 1;
        }
    
        // Get timestamp after syscall
        clock_gettime(CLOCK_MONOTONIC_RAW, &user_after);
        udelta = timespec_to_ns(user_after) - timespec_to_ns(user_before);
        kdelta = kernel_ts[1]- kernel_ts[0];
        per_call_overhead = udelta - kdelta;
        total_proc_overhead += per_call_overhead;
        // printf("%lu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n", i, 
        //     (unsigned long long)(user_before.tv_sec * 1000000000ULL + user_before.tv_nsec),
        //     (unsigned long long)(user_after.tv_sec * 1000000000ULL + user_after.tv_nsec),
        //     (unsigned long long)kernel_ts[0],(unsigned long long)kernel_ts[1], 
        //     (unsigned long long)udelta, (unsigned long long)kdelta,
        //     (unsigned long long)per_call_overhead, (unsigned long long)total_proc_overhead
        // );

        close(fd);    
    }
    
    for (size_t i = 0; i < REPS ; i++){

        fd = open(SYS_PATH, O_RDONLY);
        if (fd < 0) {
            perror("open");
            return 1;
        }
    
        // Get timestamp before syscall
        clock_gettime(CLOCK_MONOTONIC_RAW, &user_before);

        // Read the kernel-generated timestamps
        ssize_t res = read(fd, buf, sizeof(buf));
        if (res < 0) {
            perror("read");
            close(fd);
            return 1;
        }

        // Get timestamp after syscall
        clock_gettime(CLOCK_MONOTONIC_RAW, &user_after);

        // fprintf(stdout, "buf:%s\n", buf);

        buf[res] = '\0';
        unsigned long long t1, t2 = 0;
        int scanret = 0;
        scanret = sscanf(buf, "%llu %llu", &t1, &t2);
        if (scanret != 2) {
            fprintf(stderr, "Failed to parse timestamps from sysfs.\n");
            fprintf(stderr, "%llu, %llu, %s, %i\n", t1, t2, buf, scanret);
            return 1;
        }
        kernel_ts[0] = t1;
        kernel_ts[1] = t2;


        udelta = timespec_to_ns(user_after) - timespec_to_ns(user_before);
        kdelta = kernel_ts[1]- kernel_ts[0];
        per_call_overhead = udelta - kdelta;
        total_sys_overhead += per_call_overhead;
        // printf("%lu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n", i, 
        //     (unsigned long long)(user_before.tv_sec * 1000000000ULL + user_before.tv_nsec),
        //     (unsigned long long)(user_after.tv_sec * 1000000000ULL + user_after.tv_nsec),
        //     (unsigned long long)kernel_ts[0],(unsigned long long)kernel_ts[1], 
        //     (unsigned long long)udelta, (unsigned long long)kdelta,
        //     (unsigned long long)per_call_overhead, (unsigned long long)total_sys_overhead
        // );

        close(fd);    
    }
    
    

    printf("sys_call_overhead_proc_read,%lu\n",total_proc_overhead); 
    printf("sys_call_overhead_sys_read ,%lu\n",total_sys_overhead); 
        
    return 0;
}