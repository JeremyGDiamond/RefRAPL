#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

#include <linux/mm.h>
#include <asm/msr.h>
#include <linux/types.h>

//hr timer stuff
#include <linux/hrtimer.h>
#include <linux/ktime.h>

/*
    Kernel Mod that sends the 2000 most recent rapl values to a proc file
    based on the tutorial here https://medium.com/dvt-engineering/how-to-write-your-first-linux-kernel-module-cf284408beeb
    see timer docs https://www.kernel.org/doc/html/next/timers/delay_sleep_functions.html
*/

//timer wait def
#define MS_TO_WAIT 1
//size of output arr
#define MEAS_NUM 2048 //can be made config if needed
#define MEAS_MASK MEAS_NUM - 1
//buffer witten to the proc system
#define OUT_BUFF_SIZE 87 * (MEAS_NUM + 1)



struct raplMeasurement
{
    u64 ms_timestamp;
    u64 pkg, pp0, pp1, dram;
};

// arr of measuments
struct raplMeasurement measurements[MEAS_NUM];
#define MES_SIZE sizeof(measurements)

// index of next measument to write to
int mindex = 0;

// allocted here to avoid repated allocations on stack frames
// char out_buff[OUT_BUFF_SIZE];

// Module metadata
MODULE_AUTHOR("Jeremy G Diamond");
MODULE_DESCRIPTION("rapl_ref_mod");
MODULE_LICENSE("GPL");

// allocted here to avoid repated allocations on stack frames
char out_buff[OUT_BUFF_SIZE];

static struct proc_dir_entry* proc_entry;
static ssize_t custom_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    printk(KERN_INFO "printing meas");
    // header line: len = 24
    // "ts, pkg, pp0, pp1, dram\n"
    // example line: len = 87: using hex to save space
    // char *ex = "0xffffffff,0xffffffffffffffff,0xffffffffffffffff,0xffffffffffffffff,0xffffffffffffffff\n"  

    snprintf(out_buff, 25, "ts, pkg, pp0, pp1, dram\n");
    int place = 24;

    for(int i = 0; i < MEAS_NUM; ++i){
        place += snprintf(out_buff + place, 87, "%llx,%llx,%llx,%llx,%llx\n", measurements[i].ms_timestamp,measurements[i].pkg,measurements[i].pp0,measurements[i].pp1,measurements[i].dram);
    }
    int mess_length = strlen(out_buff); 
    if (*offset > 0)
        return 0; 

    long cpRes = copy_to_user(user_buffer, out_buff, mess_length);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    *offset = mess_length; 
    return mess_length;
}

static struct proc_ops pops = {
    .proc_read = custom_read
};

static struct proc_dir_entry* proc_entry1;
static ssize_t custom_read1(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    printk(KERN_INFO "bin dump meas");

    long cpRes = copy_to_user(user_buffer, measurements, MES_SIZE);
    if(cpRes != 0){
        printk(KERN_INFO "dump of meas failed");
    }
    *offset = MES_SIZE; 
    return MES_SIZE;
}

static struct proc_ops pops1 = {
    .proc_read = custom_read1
};

//hr timer
static struct hrtimer sampler;
static ktime_t interval;
u8 errorpkg, errorpp0, errorpp1, errordram;

enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
    
    measurements[mindex].ms_timestamp = ktime_to_ms(ktime_get());
    // hardcoded reg numbers for readability, see linux/arc/x86/kernel/msr.c for how to lookup
    // uses rdmsrl_safe instead of rdmsrl_safe_on_cpu assuming single cpu system
    errorpkg = rdmsrl_safe(0x611, &measurements[mindex].pkg);
    errorpp0 = rdmsrl_safe(0x639, &measurements[mindex].pp0);
    errorpp1 = rdmsrl_safe(0x641, &measurements[mindex].pp1);
    errordram = rdmsrl_safe(0x619, &measurements[mindex].dram);

    // send message to dmseg every ms when debugging: TODO ifdef this
    // printk(KERN_INFO "timer fire: %lu pkg: %llu, %i pp0: %llu, %i pp1: %llu, %i dram: %llu, %i", 
    // measurements[mindex].ms_timestamp, measurements[mindex].pkg, measurements[mindex].errorpkg, 
    // measurements[mindex].pp0, measurements[mindex].errorpp0, measurements[mindex].pp1, 
    // measurements[mindex].errorpp1, measurements[mindex].dram, measurements[mindex].errordram);
    
    // use bitwise and to mask all bits above array size
    mindex = (mindex + 1) & (MEAS_MASK);

    hrtimer_forward_now(timer, interval);
    return HRTIMER_RESTART;  // or HRTIMER_NORESTART
}


// Custom init and exit methods
static int __init custom_init(void) {
    proc_entry = proc_create("rapl_ref", 0666, NULL, &pops);
    proc_entry1 = proc_create("rapl_ref_dump", 0666, NULL, &pops1);

    interval = ktime_set(0, 1e6); // 1ms = 1,000,000 ns
    hrtimer_init(&sampler, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    sampler.function = timer_callback;
    hrtimer_start(&sampler, interval, HRTIMER_MODE_REL);
    
    printk(KERN_INFO "rapl_ref_mod loaded.");
    return 0;
}

static void __exit custom_exit(void) {
    printk(KERN_INFO "rapl_ref_mod exit");
    
    proc_remove(proc_entry);
    proc_remove(proc_entry1);
    
    hrtimer_cancel(&sampler);
    // del_timer(&sampler);
}

module_init(custom_init);

module_exit(custom_exit);