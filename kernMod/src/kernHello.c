#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h> 

// Module metadata
MODULE_AUTHOR("Jeremy G Diamond");
MODULE_DESCRIPTION("Hello world driver");
MODULE_LICENSE("GPL");

static struct proc_dir_entry* proc_entry;
static ssize_t custom_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    printk(KERN_INFO "calling our very own custom read method."); 
    char greeting[] = "Hello world!\n";
    int greeting_length = strlen(greeting); 
    if (*offset > 0)
        return 0; 

    long cpRes = copy_to_user(user_buffer, greeting, greeting_length);
    if(cpRes != 0){
        printk(KERN_INFO "copy_to_user failed !!!");
    }
    *offset = greeting_length; return greeting_length;
}

static struct proc_ops pops = {
    .proc_read = custom_read
};


// Custom init and exit methods
static int __init custom_init(void) {
    proc_entry = proc_create("helloworlddriver", 0666, NULL, &pops);
    printk(KERN_INFO "Hello world driver loaded.");
    return 0;
}

static void __exit custom_exit(void) {
    proc_remove(proc_entry);
    printk(KERN_INFO "Goodbye my friend, I shall miss you dearly...");
}

module_init(custom_init);

module_exit(custom_exit);