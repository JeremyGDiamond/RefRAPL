#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

#include <linux/mm.h>
#include <asm/msr.h>
#include <linux/types.h>

//hr timer stuff
#include <linux/ktime.h>

/*
    Kernel Mod that micro benchmarks several asm instructions, the proc file system, and the sys file system
    based on the tutorial here https://medium.com/dvt-engineering/how-to-write-your-first-linux-kernel-module-cf284408beeb
    
*/

// Module metadata
MODULE_AUTHOR("Jeremy G Diamond");
MODULE_DESCRIPTION("micro_bench_mod");
MODULE_LICENSE("GPL");


#define REPS 10000000

static struct proc_dir_entry* micro_bench_proc_dir;


static struct proc_dir_entry* rdmsr0_entry;
static ssize_t rdmsr0_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    char buffer[100];
    int buffer_length = sizeof(buffer);
    int string_buff_len = 0;
    u64 start = 0;
    u64 end = 0;
    long cpRes = 0;

    u64 rdmsr_val;
    u8 error;

    printk(KERN_INFO "micro bench rdmsr");
    
    if (*offset > 0)
        return 0; 

    start = ktime_to_ms(ktime_get());
    for (int i = 0; i < REPS; ++i){
       error = rdmsrl_safe(0x611, &rdmsr_val);
    }  
    end = ktime_to_ms(ktime_get());

    snprintf(buffer, buffer_length, "%lld,%lld", (long long)start, (long long)end);
    string_buff_len = strlen(buffer);


    cpRes = copy_to_user(user_buffer, buffer, string_buff_len);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    
    *offset = string_buff_len;
    
    return string_buff_len;
}

static struct proc_ops rdmsr0_pops = {
    .proc_read = rdmsr0_read
};


// Custom init and exit methods
static int __init custom_init(void) {
    micro_bench_proc_dir = proc_mkdir("micro_bench", NULL);
    rdmsr0_entry = proc_create("rdmsr0", 0666, micro_bench_proc_dir, &rdmsr0_pops);
    
    printk(KERN_INFO "micro_bench_mod loaded.");
    return 0;
}

static void __exit custom_exit(void) {
    printk(KERN_INFO "micro_bench_mod exit");
    
    proc_remove(rdmsr0_entry);
    
    // must be last
    proc_remove(micro_bench_proc_dir);
    
}

module_init(custom_init);

module_exit(custom_exit);