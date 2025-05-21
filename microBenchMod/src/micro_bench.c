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


static struct proc_dir_entry* proc_entry;
static ssize_t custom_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    char greeting[] = "Hello world!\n";
    int greeting_length = strlen(greeting);

    printk(KERN_INFO "micro bench rdmsr");
    
    if (*offset > 0)
        return 0; 
    
    copy_to_user(user_buffer, greeting, greeting_length);
    
    *offset = greeting_length;
    
    return greeting_length;
}

static struct proc_ops pops = {
    .proc_read = custom_read
};


// Custom init and exit methods
static int __init custom_init(void) {
    proc_entry = proc_create("rapl_ref", 0666, NULL, &pops);
    
    printk(KERN_INFO "micro_bench_mod loaded.");
    return 0;
}

static void __exit custom_exit(void) {
    printk(KERN_INFO "micro_bench_mod exit");
    
    proc_remove(proc_entry);
    
}

module_init(custom_init);

module_exit(custom_exit);