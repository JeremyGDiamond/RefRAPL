#define _GNU_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <fcntl.h>

//pid must be global to use in sig_int_hand
static volatile pid_t pid = 0;
void sig_int_hand(int sig){
    if (sig == SIGINT){
        if (!pid){
            printf("INFO: End Samples\n");
            exit(0);
        }
        else{
            // wait for child, unload mod
            usleep(1000*100);
            int sysRet = system("rmmod msr");

            if (sysRet == 0) {
                printf("INFO: MSR module unloaded\n");
            } 
            
            else {
                printf("ERROR: Failed to unload MSR module\n");
            }

        }
    }
}


int main(int argc, char** argv) {
    
    /*
    INPUT ARGS IN ORDER:
        process to run: 2048 char max
        name of file to write to: 2048 byte buffer
    */

    /*
    check the input args
    check if we have sudo priv
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

    // check privs
    if (geteuid() != 0){
        printf("ERROR: This sensor needs sudo privs\n");
        return -1;
    }

    // check if module is loaded, try to load once
    
    struct stat sb;
    
    if (!(stat("/sys/module/msr", &sb) == 0 && S_ISDIR(sb.st_mode))) {
        printf("INFO: MSR module not already loaded\n");

        int sysRet = system("modprobe msr");

        if (sysRet == 0) {
            printf("INFO: MSR module loaded, On successful runs it will be unloaded\n");
        } 
        
        else {
            printf("ERROR: Failed to load MSR module\n");
            return -1;
        }

    }
    }
    
    // fork process
    pid = fork();

    if (pid < 0){
        printf("ERROR: Failed to fork process c1\n");
            return -1;
    }

    // child process 1: take a sample every milisecond, write to file every 100 samples
    // we use 5 sepserate files and buffers to keep things fast in this loop
    else if (pid == 0){
        
        //make the fnames
        char pkgname[2048];
        char pp0name[2048];
        char pp1name[2048];
        char drmname[2048];
        char timname[2048];

        snprintf(pkgname, 2048, "%s_%s.data", argv[2], "pkg");
        snprintf(pp0name, 2048, "%s_%s.data", argv[2], "pp0");
        snprintf(pp1name, 2048, "%s_%s.data", argv[2], "pp1");
        snprintf(drmname, 2048, "%s_%s.data", argv[2], "drm");
        snprintf(timname, 2048, "%s_%s.data", argv[2], "tim");
        
        printf("INFO: child start\n");
        // open the files and allocate data
        char msr_file[64] = "/dev/cpu/0/msr";
        int fdmsr = open(msr_file, O_RDONLY);
        FILE *fpkg = fopen(pkgname, "w");
        FILE *fpp0 = fopen(pp0name, "w");
        FILE *fpp1 = fopen(pp1name, "w");
        FILE *fdrm = fopen(drmname, "w");
        FILE *fts = fopen(timname, "w");
        
        
        // we assume cpu0 exists, and supports msrs there should be error handeling here. See msr-tools/rdmsr.c line 218
        // time stamp in microseconds
        struct timeval mts[100];
        // RAPL mes reg are 64 bit values as can be found in the intel dev manuel page 3631
        // msr number assumed to match dev's cpu for now TODO: make this an input arg or a lookup by cpuid
        u_int32_t msr_pkg_num = 0x611;
        u_int32_t msr_pp0_num = 0x639;
        u_int32_t msr_pp1_num = 0x641;
        u_int32_t msr_dram_num = 0x619;
        // buffers use msr names
        u_int64_t msr_pkg_energy_status[100], msr_pp0_energy_status[100], msr_pp1_energy_status[100], msr_dram_energy_status[100];
        int placeholder = 0;
        

        while(1){
            // printf("INFO: child takes mesurment\n");
            // read the msrs and add them to a buffer every milisecond in a 100 sample for loop
            // we assume the read works without error handleing. See msr-tools/rdmsr.c line 235
            
            
            for(size_t i = 0; i < 100; ++i){
                placeholder = pread(fdmsr, &msr_pkg_energy_status[i], sizeof msr_pkg_energy_status[i], msr_pkg_num);
                placeholder = pread(fdmsr, &msr_pp0_energy_status[i], sizeof msr_pp0_energy_status[i], msr_pp0_num);
                placeholder = pread(fdmsr, &msr_pp1_energy_status[i], sizeof msr_pp1_energy_status[i], msr_pp1_num);
                placeholder = pread(fdmsr, &msr_dram_energy_status[i], sizeof msr_dram_energy_status[i], msr_dram_num);
                gettimeofday(&mts[i], NULL);
                
                //TODO: ifdef this print
                // printf("INFO: child print last meas, %ld: %lu, %lu, %lu, %lu\n", (mts[i].tv_sec * 1000000) + mts[i].tv_usec, msr_pkg_energy_status[i], msr_pp0_energy_status[i], 
                //         msr_pp1_energy_status[i], msr_dram_energy_status[i]);
                
                usleep(1000);
            }
            // printf("INFO: child print last meas, %ld: %lu, %lu, %lu, %lu\n", (mts.tv_sec * 1000000) + mts.tv_usec, msr_pkg_energy_status[99], msr_pp0_energy_status[99], 
            //             msr_pp1_energy_status[99], msr_dram_energy_status[99]);
            // append buffer to file
            fwrite(msr_pkg_energy_status, sizeof(msr_pkg_energy_status[0]), 100, fpkg);
            fwrite(msr_pp0_energy_status, sizeof(msr_pp0_energy_status[0]), 100, fpp0);
            fwrite(msr_pp1_energy_status, sizeof(msr_pp1_energy_status[0]), 100, fpp1);
            fwrite(msr_dram_energy_status, sizeof(msr_dram_energy_status[0]), 100, fdrm);
            fwrite(mts, sizeof(mts[0]), 100, fts);
        }
        placeholder = 0;
        
        //close the files
        fclose(fpkg);
        fclose(fpp0);
        fclose(fpp1);
        fclose(fdrm);
        fclose(fts);

        exit(placeholder);
        
    }

    
    // parent process: wait 1 sec and then fork again run the target process, only user privs
    // when process undr test is done sigint to child 1
    else{
        usleep(1000 * 1000);
    
        printf("INFO: run process\n");
        
        struct timeval t1,t2,t3,t4;
        int placeholder = 0;
        
        // placeholder = setuid(1000);
        // take ts for pre run overhead
        gettimeofday(&t1, NULL);
        // sleep 1 sec
        usleep(1000*1000);
        // record time stamp start
        gettimeofday(&t2, NULL);
        placeholder = system(argv[1]);
        gettimeofday(&t3, NULL);
        // record time stamp end
        // sleep 1 sec
        usleep(1000*1000);
        // take ts for post run overhead
        gettimeofday(&t4, NULL);



        printf("INFO: process ret: %u t1: %ld, t2: %ld, t3: %ld, t4: %ld\n", placeholder, 
                (t1.tv_sec * 1000000) + t1.tv_usec, 
                (t2.tv_sec * 1000000) + t2.tv_usec, 
                (t3.tv_sec * 1000000) + t3.tv_usec, 
                (t4.tv_sec * 1000000) + t4.tv_usec);
        
        // write to file

        char ptsname[2048];
        snprintf(ptsname, 2048, "%s_%s.data", argv[2], "pts");
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