    #define _GNU_SOURCE 

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <string.h>
    #include <sys/wait.h>
    #include <signal.h>
    #include <errno.h>
    #include <sys/time.h>
    #include <fcntl.h>

    int main(int argc, char** argv) {
        
        /*
        INPUT ARGS IN ORDER:
            process to run: 2048 char max
            name of file to write to: 2048 byte buffer
        */

        /*
        check if we have sudo priv
        check the input args
        check if the kernel mod is loaded
        start measurments every milisecond in another process
        launch another program
        check if it has terminated every 10th of a second
        term the measuemnt process
        */

        // checking env, args and turning on msr kernel mod, extra block to fold code away
        {   
        // check arg number and len
        if (argc != 3){
            printf("ERROR: wrong number of args\n");
            return -1;
        }

        if (strlen(argv[1]) > 2048){
            printf("ERROR: process name too long\n");
            return -1;
        }

        if (strlen(argv[2]) > 2048){
            printf("ERROR: file name too long\n");
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
        pid_t pid = fork();

        if (pid < 0){
            printf("ERROR: Failed to fork process c1\n");
                return -1;
        }

        // child process 1: take a sample every milisecond, check if the process is done every 100 samples
        else if (pid == 0){
            
            
            printf("INFO: child start\n");
            // open the file and allocate data
            char msr_file[64] = "/dev/cpu/0/msr";
            int fd = open(msr_file, O_RDONLY);
            // we assume cpu0 exists, and supports msrs there should be error handeling here. See msr-tools/rdmsr.c line 218
            // time stamp in microseconds
            struct timeval mts;
            // RAPL mes reg are 32 bit values as can be found in the intel dev manuel page 3631
            // msr number assumed to match dev's cpu for now TODO: make this an input arg or a lookup by cpuid
            u_int32_t msr_pkg_num = 0x611;
            u_int32_t msr_pp0_num = 0x639;
            u_int32_t msr_pp1_num = 0x641;
            u_int32_t msr_dram_num = 0x619;
            // buffers use msr names
            u_int64_t MSR_PKG_ENERGY_STATUS[100], MSR_PP0_ENERGY_STATUS[100], MSR_PP1_ENERGY_STATUS[100], MSR_DRAM_ENERGY_STATUS[100];
            int placeholder = 0;
            

            while(1){
                printf("INFO: child takes mesurment\n");
                // read the msrs and add them to a buffer every milisecond in a 100 sample for loop
                // we assume the read works without error handleing. See msr-tools/rdmsr.c line 235
                
                
                for(size_t i = 0; i < 100; ++i){
                    placeholder = pread(fd, &MSR_PKG_ENERGY_STATUS[i], sizeof MSR_PKG_ENERGY_STATUS[i], msr_pkg_num);
                    placeholder = pread(fd, &MSR_PP0_ENERGY_STATUS[i], sizeof MSR_PKG_ENERGY_STATUS[i], msr_pp0_num);
                    placeholder = pread(fd, &MSR_PP1_ENERGY_STATUS[i], sizeof MSR_PKG_ENERGY_STATUS[i], msr_pp1_num);
                    placeholder = pread(fd, &MSR_DRAM_ENERGY_STATUS[i], sizeof MSR_PKG_ENERGY_STATUS[i], msr_dram_num);
                    gettimeofday(&mts, NULL);
                    // printf("INFO: child print last meas, %ld: %lu, %lu, %lu, %lu\n", (mts.tv_sec * 1000000) + mts.tv_usec, MSR_PKG_ENERGY_STATUS[i], MSR_PP0_ENERGY_STATUS[i], 
                    //         MSR_PP1_ENERGY_STATUS[i], MSR_DRAM_ENERGY_STATUS[i]);
                    usleep(1000);
                }
                printf("INFO: child print last meas, %ld: %lu, %lu, %lu, %lu\n", (mts.tv_sec * 1000000) + mts.tv_usec, MSR_PKG_ENERGY_STATUS[99], MSR_PP0_ENERGY_STATUS[99], 
                            MSR_PP1_ENERGY_STATUS[99], MSR_DRAM_ENERGY_STATUS[99]);
                // append buffer to file
            }
            placeholder = 0;
            // COOP sigint handeler closes the file

            exit(placeholder);
            
        }

        
        // parent process: wait 1 sec and then fork again run the target process, only user privs
        // when child 2 done sigint to child 1
        else{
            sleep(3);

            // fork process
            pid_t pidc2 = fork();
            int status = 0;

            if (pidc2 < 0){
                printf("ERROR: Failed to fork process c1\n");
                    return -1;
            }

            // child process 2: setuid and run a system command
            else if (pidc2 == 0){
                printf("INFO: run child 2\n");
                // time_t t1,t2,t3,t4 = 0;

                struct timeval t1,t2,t3,t4;
                int placeholder = 0;
                
                placeholder = setuid(1000);
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



                printf("INFO:child 2 ret: %u t1: %ld, t2: %ld, t3: %ld, t4: %ld\n", placeholder, 
                        (t1.tv_sec * 1000000) + t1.tv_usec, 
                        (t2.tv_sec * 1000000) + t2.tv_usec, 
                        (t3.tv_sec * 1000000) + t3.tv_usec, 
                        (t4.tv_sec * 1000000) + t4.tv_usec);
                
                // COOP write to file


            }

            else{
                waitpid(pidc2, &status, 0);

                usleep(1000*100);

                kill(pid, SIGINT);

                // unload kernel mod COOP: do this on sigint

               

                int sysRet = system("rmmod msr");

                if (sysRet == 0) {
                    printf("INFO: MSR module unloaded\n");
                } 
                
                else {
                    printf("ERROR: Failed to unload MSR module\n");
                    return -1;
                }
            }
            
            
        }
        

        return 0;
    }