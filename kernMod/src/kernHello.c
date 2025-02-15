#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/delay.h> 

// Module metadata
MODULE_AUTHOR("Jeremy G Diamond");
MODULE_DESCRIPTION("Hello world driver");
MODULE_LICENSE("GPL");

static void func(void){
    while(1){
        printk(KERN_INFO "Hello world driver running.");
        fsleep(1000 * 1000);
    }
    return;
}

// Custom init and exit methods
static int __init custom_init(void) {
    printk(KERN_INFO "Hello world driver loaded.");
    // func(); // THIS IS FUCKED DO NOT DO THIS!!!!!!
    return 0;
}

static void __exit custom_exit(void) {
    printk(KERN_INFO "Goodbye my friend, I shall miss you dearly...");
}

module_init(custom_init);

module_exit(custom_exit);