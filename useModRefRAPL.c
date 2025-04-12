#define _GNU_SOURCE 

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

#define PROC_PATH "/proc/rapl_ref_dump"

// struct to b used in dataread
struct raplMeasurement {
    uint64_t ms_timestamp;
    uint8_t errorpkg, errorpp0, errorpp1, errordram;
    uint64_t pkg, pp0, pp1, dram;
};

uint64_t get_monotonic_raw_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

//pid must be global to use in sig_int_hand
static volatile pid_t pid = 0;
void sig_int_hand(int sig){
    if (sig == SIGINT){
        if (!pid){
            printf("INFO: End Samples\n");
            exit(0);

        }
        else{
            // wait for child
            usleep(1000*100);

            // NOTE: mod can be left for now but this is how you would do it
            // int sysRet = system("rmmod rapl_ref");

            // if (sysRet == 0) {
            //     printf("INFO: RAPL_Ref module unloaded\n");
            // } 
            
            // else {
            //     printf("ERROR: Failed to unload RAPL_Ref module\n");
            // }

        }
    }
}


int main(int argc, char** argv) {
    
    /*
    INPUT ARGS IN ORDER:
        process to run: 2039 char max
        name of file to write to: 2039 byte buffer
    */

    /*
    check the input args
    check if the kernel mod is loaded
    start measurments every milisecond in another process
    launch another program
    check if it has terminated every 10th of a second
    term the measuemnt process
    */

    // add sig_int_handel for process shutdown
    signal(SIGINT, sig_int_hand);

    // checking env, args and turning on msr kernel mod, extra block to fold code away
    {   
    // check arg number and len
    if (argc != 3){
        printf("ERROR: wrong number of args\n");
        return -1;
    }

    // data files add 8 chars, 2048 - 8 - 1 for the null at the end of the string
    if (strlen(argv[1]) > 2039){
        printf("ERROR: process name too long: max 2039\n");
        return -1;
    }

    if (strlen(argv[2]) > 2039){
        printf("ERROR: file name too long: max 2039\n");
        return -1;
    }


    // check if module is loaded
    
    if (access(PROC_PATH, F_OK) != 0){
        printf("ERROR: file %s not availible, Load the rapl_ref Mod to fix this\n", PROC_PATH);
    }
    }
    
    // fork process
    pid = fork();

    if (pid < 0){
        printf("ERROR: Failed to fork process c1\n");
            return -1;
    }

    // child process 1: take a sample from /proc/rapl_ref_dump every 2 seconds
    // dump size is sizeof rapl_measurment * ring buffer size
    // append it to filname.raw, process in userspace later
    else if (pid == 0){
        // define macros and set const for reads
        char out_file[2048];
        snprintf(out_file, 2048, "data/%s.data", argv[2]);
        int meas_size = sizeof(struct raplMeasurement); 
        #define STRUCT_COUNT 2048
        #define CHUNK_SIZE (meas_size * STRUCT_COUNT)
        #define WAIT_TIME_NS 2000*1000

        char buffer[CHUNK_SIZE];
        int proc_fd = 0;
        int out_fd = 0;
        ssize_t bytes_read = 0;
        

        while(1){
            // printf("INFO: child takes mesurment\n");
            // append to dump every 2 seconds
            {
            proc_fd = open(PROC_PATH, O_RDONLY);
            if (proc_fd < 0) {
                printf("Error: Failed to open proc file");
                break;
            }

            bytes_read = read(proc_fd, buffer, CHUNK_SIZE);
            close(proc_fd);

            if (bytes_read != CHUNK_SIZE) {
                printf("Warning: expected %d bytes, got %zd\n", CHUNK_SIZE, bytes_read);
                continue;
            }

            out_fd = open(out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (out_fd < 0) {
                printf("Error: Failed to open output file");
                break;
            }

            if (write(out_fd, buffer, bytes_read) != bytes_read) {
                printf("Error: Failed to write to output file");
                close(out_fd);
                break;
            }

            close(out_fd);
            }
            usleep(WAIT_TIME_NS);
            
        }

        // append dump to file one last time
        {
        proc_fd = open(PROC_PATH, O_RDONLY);
        if (proc_fd < 0) {
            printf("Error: Failed to open proc file");
        }

        bytes_read = read(proc_fd, buffer, CHUNK_SIZE);
        close(proc_fd);

        if (bytes_read != CHUNK_SIZE) {
            printf("Warning: expected %d bytes, got %zd\n", CHUNK_SIZE, bytes_read);
        }

        out_fd = open(out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (out_fd < 0) {
            printf("Error: Failed to open output file");
        }

        if (write(out_fd, buffer, bytes_read) != bytes_read) {
            printf("Error: Failed to write to output file");
            close(out_fd);
        }

        close(out_fd);
        }
    
        exit(0);
        
    }

    
    // parent process: wait 1 sec and then fork again run the target process, only user privs
    // when process undr test is done sigint to child 1
    else{
        #define PAR_WAIT_NS 1000*1000

        usleep(PAR_WAIT_NS);
    
        printf("INFO: run process\n");
        
        uint64_t t1,t2,t3,t4;
        int placeholder = 0;
        
        // take ts for pre run overhead
        t1 = get_monotonic_raw_ns();
        // sleep 1 sec
        usleep(PAR_WAIT_NS);
        // record time stamp start
        t2 = get_monotonic_raw_ns();
        placeholder = system(argv[1]);
        t3 = get_monotonic_raw_ns();
        // record time stamp end
        // sleep 1 sec
        usleep(PAR_WAIT_NS);
        // take ts for post run overhead
        t4 = get_monotonic_raw_ns();

        printf("INFO: process ret: %u t1: %ld, t2: %ld, t3: %ld, t4: %ld\n", placeholder, t1,t2,t3,t4);
        
        // write to file

        char ptsname[2048];
        snprintf(ptsname, 2048, "data/%s%s.data", argv[2], "pts");
        FILE *fprocts = fopen(ptsname, "w");
        
        fwrite(&t1, sizeof(t1), 1, fprocts);
        fwrite(&t2, sizeof(t2), 1, fprocts);
        fwrite(&t3, sizeof(t3), 1, fprocts);
        fwrite(&t4, sizeof(t4), 1, fprocts);
        fclose(fprocts);

        usleep(1000*100);

        kill(pid, SIGINT);

        // shutdown procedure, call the sigint function directly
        sig_int_hand(SIGINT);
        
    }
    

    return 0;
}