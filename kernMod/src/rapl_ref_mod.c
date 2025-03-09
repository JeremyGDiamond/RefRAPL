#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h> 
#include <linux/slab.h>
#include <linux/mm.h>
#include <asm/msr.h>

/*
    Kernel Mod that sends the 2000 most recent rapl values to a proc file
    based on the tutorial here https://medium.com/dvt-engineering/how-to-write-your-first-linux-kernel-module-cf284408beeb
    see timer docs https://www.kernel.org/doc/html/next/timers/delay_sleep_functions.html
*/

//timer wait def
#define MS_TO_WAIT 1
//size of output arr
#define MEAS_NUM 2000
//buffer witten to the proc system
#define OUT_BUFF_SIZE 87 * (MEAS_NUM + 1)

struct raplMeasurement
{
    unsigned long ms_timestamp;
    int errorpkg, errorpp0, errorpp1, errordram;
    unsigned long long pkg, pp0, pp1, dram;
};

// arr of measuments
struct raplMeasurement measurements[MEAS_NUM];

// index of next measument to write to
int mindex = 0;

// allocted here to avoid repated allocations on stack frames
char out_buff[OUT_BUFF_SIZE];
unsigned long old_jiffies;

// Module metadata
MODULE_AUTHOR("Jeremy G Diamond");
MODULE_DESCRIPTION("rapl_ref_mod");
MODULE_LICENSE("GPL");

static struct proc_dir_entry* proc_entry;
static ssize_t custom_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    printk(KERN_INFO "printing meas");
    // header line: len = 24
    // "ts, pkg, pp0, pp1, dram\n"
    // example line: len = 87: using hex to save space
    // char *ex = "0xffffffff,0xffffffffffffffff,0xffffffffffffffff,0xffffffffffffffff,0xffffffffffffffff\n"  

    snprintf(out_buff, 24, "ts, pkg, pp0, pp1, dram\n");
    int place = 23;

    for(int i = 0; i < MEAS_NUM; ++i){
        place += snprintf(out_buff + place, 87, "%lx,%llx,%llx,%llx,%llx\n", measurements[i].ms_timestamp,measurements[i].pkg,measurements[i].pp0,measurements[i].pp1,measurements[i].dram);
    }
    int mess_length = strlen(out_buff); 
    if (*offset > 0)
        return 0; 

    long cpRes = copy_to_user(user_buffer, out_buff, mess_length);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    *offset = mess_length; return mess_length;
}

static struct proc_ops pops = {
    .proc_read = custom_read
};


static struct timer_list sampler;

void timer_callback(struct timer_list *timer){
    old_jiffies = jiffies;
    measurements[mindex].ms_timestamp = jiffies_to_msecs(old_jiffies);
    // hardcoded reg numbers for readability, see linux/arc/x86/kernel/msr.c for how to lookup
    // uses rdmsrl_safe instead of rdmsrl_safe_on_cpu assuming single cpu system
    measurements[mindex].errorpkg = rdmsrl_safe(0x611, &measurements[mindex].pkg);
    measurements[mindex].errorpp0 = rdmsrl_safe(0x639, &measurements[mindex].pp0);
    measurements[mindex].errorpp1 = rdmsrl_safe(0x641, &measurements[mindex].pp1);
    measurements[mindex].errordram = rdmsrl_safe(0x619, &measurements[mindex].dram);

    // send message to dmseg every ms when debugging: TODO ifdef this
    // printk(KERN_INFO "timer fire: %lu pkg: %llu, %i pp0: %llu, %i pp1: %llu, %i dram: %llu, %i", 
    // measurements[mindex].ms_timestamp, measurements[mindex].pkg, measurements[mindex].errorpkg, 
    // measurements[mindex].pp0, measurements[mindex].errorpp0, measurements[mindex].pp1, 
    // measurements[mindex].errorpp1, measurements[mindex].dram, measurements[mindex].errordram);
    
    mindex = (mindex + 1) % 2000;
    // use old_jiffies to account for time to read msr
    mod_timer(timer, old_jiffies + msecs_to_jiffies(MS_TO_WAIT));
}


// Custom init and exit methods
static int __init custom_init(void) {
    proc_entry = proc_create("rapl_ref", 0666, NULL, &pops);
    timer_setup(&sampler, timer_callback, 0);
    mod_timer(&sampler, jiffies + msecs_to_jiffies(MS_TO_WAIT));

    printk(KERN_INFO "rapl_ref_mod loaded.");
    return 0;
}

static void __exit custom_exit(void) {
    proc_remove(proc_entry);
    del_timer(&sampler);
    printk(KERN_INFO "rapl_ref_mod exit");
}

module_init(custom_init);

module_exit(custom_exit);