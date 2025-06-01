#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

#include <linux/sysfs.h>
#include <linux/kobject.h>

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


#define REPS 100000

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
    u32 low, high;

    if (*offset > 0)
        return 0;

    start = ktime_to_ns(ktime_get());
    for (int i = 0; i < REPS; ++i){
        // Inline asm to read MSR 0x611 into low/high parts
        asm volatile(
            "rdmsr"
            : "=a"(low), "=d"(high)
            : "c"(0x611)
        );
        rdmsr_val = ((u64)high << 32) | low;
    }
    end = ktime_to_ns(ktime_get());

    snprintf(buffer, buffer_length, "rdmsr 0x611                ,%lld,%lld,%lld\n", (long long)(end-start), (long long)start, (long long)end);
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

static struct proc_dir_entry* rdmsr1_entry;
static ssize_t rdmsr1_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    char buffer[100];
    int buffer_length = sizeof(buffer);
    int string_buff_len = 0;
    u64 start = 0;
    u64 end = 0;
    long cpRes = 0;

    u64 rdmsr_val;
    u32 low, high;

    if (*offset > 0)
        return 0;

    start = ktime_to_ns(ktime_get());
    for (int i = 0; i < REPS; ++i){
        // Inline asm to read MSR 0x611 into low/high parts
        asm volatile(
            "rdmsr"
            : "=a"(low), "=d"(high)
            : "c"(0x639)
        );
        rdmsr_val = ((u64)high << 32) | low;
    }
    end = ktime_to_ns(ktime_get());

    snprintf(buffer, buffer_length, "rdmsr 0x639                ,%lld,%lld,%lld\n", (long long)(end-start), (long long)start, (long long)end);
    string_buff_len = strlen(buffer);


    cpRes = copy_to_user(user_buffer, buffer, string_buff_len);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    
    *offset = string_buff_len;
    
    return string_buff_len;
}

static struct proc_ops rdmsr1_pops = {
    .proc_read = rdmsr1_read
};




static struct proc_dir_entry* rdmsr2_entry;
static ssize_t rdmsr2_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    char buffer[100];
    int buffer_length = sizeof(buffer);
    int string_buff_len = 0;
    u64 start = 0;
    u64 end = 0;
    long cpRes = 0;

    u64 rdmsr_val;
    u32 low, high;

    if (*offset > 0)
        return 0;

    start = ktime_to_ns(ktime_get());
    for (int i = 0; i < REPS; ++i){
        // Inline asm to read MSR 0x611 into low/high parts
        asm volatile(
            "rdmsr"
            : "=a"(low), "=d"(high)
            : "c"(0x641)
        );
        rdmsr_val = ((u64)high << 32) | low;
    }
    end = ktime_to_ns(ktime_get());

    snprintf(buffer, buffer_length, "rdmsr 0x641                ,%lld,%lld,%lld\n", (long long)(end-start), (long long)start, (long long)end);
    string_buff_len = strlen(buffer);


    cpRes = copy_to_user(user_buffer, buffer, string_buff_len);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    
    *offset = string_buff_len;
    
    return string_buff_len;
}

static struct proc_ops rdmsr2_pops = {
    .proc_read = rdmsr2_read
};




static struct proc_dir_entry* rdmsr3_entry;
static ssize_t rdmsr3_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    char buffer[100];
    int buffer_length = sizeof(buffer);
    int string_buff_len = 0;
    u64 start = 0;
    u64 end = 0;
    long cpRes = 0;

    u64 rdmsr_val;
    u32 low, high;

    if (*offset > 0)
        return 0;

    start = ktime_to_ns(ktime_get());
    for (int i = 0; i < REPS; ++i){
        // Inline asm to read MSR 0x611 into low/high parts
        asm volatile(
            "rdmsr"
            : "=a"(low), "=d"(high)
            : "c"(0x619)
        );
        rdmsr_val = ((u64)high << 32) | low;
    }
    end = ktime_to_ns(ktime_get());

    snprintf(buffer, buffer_length, "rdmsr 0x619                ,%lld,%lld,%lld\n", (long long)(end-start), (long long)start, (long long)end);
    string_buff_len = strlen(buffer);


    cpRes = copy_to_user(user_buffer, buffer, string_buff_len);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    
    *offset = string_buff_len;
    
    return string_buff_len;
}

static struct proc_ops rdmsr3_pops = {
    .proc_read = rdmsr3_read
};




static struct proc_dir_entry* rdmsr4_entry;
static ssize_t rdmsr4_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    char buffer[100];
    int buffer_length = sizeof(buffer);
    int string_buff_len = 0;
    u64 start = 0;
    u64 end = 0;
    long cpRes = 0;

    u64 rdmsr_val;
    u32 low, high;

    if (*offset > 0)
        return 0;

    start = ktime_to_ns(ktime_get());
    for (int i = 0; i < REPS; ++i){
        // Inline asm to read MSR 0x611 into low/high parts
        asm volatile(
            "rdmsr"
            : "=a"(low), "=d"(high)
            : "c"(0x19C)
        );
        rdmsr_val = ((u64)high << 32) | low;
    }
    end = ktime_to_ns(ktime_get());

    snprintf(buffer, buffer_length, "rdmsr 0x19C                ,%lld,%lld,%lld\n", (long long)(end-start), (long long)start, (long long)end);
    string_buff_len = strlen(buffer);


    cpRes = copy_to_user(user_buffer, buffer, string_buff_len);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    
    *offset = string_buff_len;
    
    return string_buff_len;
}

static struct proc_ops rdmsr4_pops = {
    .proc_read = rdmsr4_read
};




static struct proc_dir_entry* rdmsr5_entry;
static ssize_t rdmsr5_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    char buffer[100];
    int buffer_length = sizeof(buffer);
    int string_buff_len = 0;
    u64 start = 0;
    u64 end = 0;
    long cpRes = 0;

    u64 rdmsr_val;
    u32 low, high;

    if (*offset > 0)
        return 0;

    start = ktime_to_ns(ktime_get());
    for (int i = 0; i < REPS; ++i){
        // Inline asm to read MSR 0x611 into low/high parts
        asm volatile(
            "rdmsr"
            : "=a"(low), "=d"(high)
            : "c"(0x17)
        );
        rdmsr_val = ((u64)high << 32) | low;
    }
    end = ktime_to_ns(ktime_get());

    snprintf(buffer, buffer_length, "rdmsr 0x17                 ,%lld,%lld,%lld\n", (long long)(end-start), (long long)start, (long long)end);
    string_buff_len = strlen(buffer);


    cpRes = copy_to_user(user_buffer, buffer, string_buff_len);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    
    *offset = string_buff_len;
    
    return string_buff_len;
}

static struct proc_ops rdmsr5_pops = {
    .proc_read = rdmsr5_read
};

static struct proc_dir_entry* cpuid_entry;
static ssize_t cpuid_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    char buffer[100];
    int buffer_length = sizeof(buffer);
    int string_buff_len = 0;
    u64 start = 0;
    u64 end = 0;
    long cpRes = 0;

    u32 eax, ebx, ecx, edx;

    // printk(KERN_INFO "micro bench cpuid");
    
    if (*offset > 0)
        return 0; 

    start = ktime_to_ns(ktime_get());
    for (int i = 0; i < REPS; ++i){
        eax = 0x1;
        ecx = 0x0;
        asm volatile("cpuid"
                     : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                     : "a" (eax), "c" (ecx));
    }  
    end = ktime_to_ns(ktime_get());

    snprintf(buffer, buffer_length, "cpuid 0x1 0                ,%lld,%lld,%lld\n", (long long)(end-start), (long long)start, (long long)end);
    string_buff_len = strlen(buffer);


    cpRes = copy_to_user(user_buffer, buffer, string_buff_len);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    
    *offset = string_buff_len;
    
    return string_buff_len;
}

static struct proc_ops cpuid_pops = {
    .proc_read = cpuid_read
};

static struct proc_dir_entry* mov_entry;
static ssize_t mov_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    char buffer[100];
    int buffer_length = sizeof(buffer);
    int string_buff_len = 0;
    u64 start = 0;
    u64 end = 0;
    long cpRes = 0;

    u32 eax, ebx;

    // printk(KERN_INFO "micro bench mov");
    
    if (*offset > 0)
        return 0; 

    start = ktime_to_ns(ktime_get());
    for (int i = 0; i < REPS; ++i){
        asm volatile("movl %1, %0" : "=r"(eax) : "r"(ebx));
    }  
    end = ktime_to_ns(ktime_get());

    snprintf(buffer, buffer_length, "mov eax ebx                ,%lld,%lld,%lld\n", (long long)(end-start), (long long)start, (long long)end);
    string_buff_len = strlen(buffer);


    cpRes = copy_to_user(user_buffer, buffer, string_buff_len);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    
    *offset = string_buff_len;
    
    return string_buff_len;
}

static struct proc_ops mov_pops = {
    .proc_read = mov_read
};

static struct proc_dir_entry* nop_entry;
static ssize_t nop_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{
    char buffer[100];
    int buffer_length = sizeof(buffer);
    int string_buff_len = 0;
    u64 start = 0;
    u64 end = 0;
    long cpRes = 0;

    // printk(KERN_INFO "micro bench nop");
    
    if (*offset > 0)
        return 0; 

    start = ktime_to_ns(ktime_get());
    for (int i = 0; i < REPS; ++i){
        asm volatile("nop");
    }  
    end = ktime_to_ns(ktime_get());

    snprintf(buffer, buffer_length, "nop                        ,%lld,%lld,%lld\n", (long long)(end-start), (long long)start, (long long)end);
    string_buff_len = strlen(buffer);


    cpRes = copy_to_user(user_buffer, buffer, string_buff_len);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    
    *offset = string_buff_len;
    
    return string_buff_len;
}

static struct proc_ops nop_pops = {
    .proc_read = nop_read
};

static struct proc_dir_entry* sys_call_overhead_proc_entry;
static ssize_t sys_call_overhead_proc_read(struct file* file, char __user* user_buffer, size_t count, loff_t* offset)
{

    u64 ts[2];
    int ts_size = sizeof(u64)*2;
    long cpRes = 0;
    u32 low, high;

    // printk(KERN_INFO "micro bench sys_call_overhead_proc");
    
    if (*offset > 0)
        return 0; 

    ts[0] = ktime_to_ns(ktime_get());
    
    asm volatile(
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(0x17)
    );
      
    ts[1] = ktime_to_ns(ktime_get());

    cpRes = copy_to_user(user_buffer, ts, sizeof(u64)*2);
    if(cpRes != 0){
        printk(KERN_INFO "print of meas failed");
    }
    
    *offset = ts_size;
    
    return ts_size;
}

static struct proc_ops sys_call_overhead_proc_pops = {
    .proc_read = sys_call_overhead_proc_read
};

static struct kobject *syscall_kobj;

static ssize_t sys_call_overhead_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    u64 ts[2];
    u32 low, high;
    u64 msr_value;
    
    ts[0] = ktime_to_ns(ktime_get());

    // Read MSR 0x17
    asm volatile(
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(0x17)
    );

    ts[1] = ktime_to_ns(ktime_get());

    return scnprintf(buf, PAGE_SIZE, "%llu %llu\n", ts[0], ts[1]);
}

static struct kobj_attribute syscall_overhead_attr = __ATTR_RO(sys_call_overhead);


// Custom init and exit methods
static int __init custom_init(void) {
    int err = 0;

    micro_bench_proc_dir = proc_mkdir("micro_bench", NULL);
    rdmsr0_entry = proc_create("rdmsr0", 0666, micro_bench_proc_dir, &rdmsr0_pops);
    rdmsr1_entry = proc_create("rdmsr1", 0666, micro_bench_proc_dir, &rdmsr1_pops);
    rdmsr2_entry = proc_create("rdmsr2", 0666, micro_bench_proc_dir, &rdmsr2_pops);
    rdmsr3_entry = proc_create("rdmsr3", 0666, micro_bench_proc_dir, &rdmsr3_pops);
    rdmsr4_entry = proc_create("rdmsr4", 0666, micro_bench_proc_dir, &rdmsr4_pops);
    rdmsr5_entry = proc_create("rdmsr5", 0666, micro_bench_proc_dir, &rdmsr5_pops);

    cpuid_entry = proc_create("cpuid", 0666, micro_bench_proc_dir, &cpuid_pops);
    mov_entry = proc_create("mov", 0666, micro_bench_proc_dir, &mov_pops);
    nop_entry = proc_create("nop", 0666, micro_bench_proc_dir, &nop_pops);

    sys_call_overhead_proc_entry = proc_create("sys_call_overhead_proc", 0666, micro_bench_proc_dir, &sys_call_overhead_proc_pops);

    syscall_kobj = kobject_create_and_add("syscall_bench", kernel_kobj);
    if (!syscall_kobj)
        return -ENOMEM;

    err = sysfs_create_file(syscall_kobj, &syscall_overhead_attr.attr);
    if (err) {
        kobject_put(syscall_kobj);
        return err;
    }
    
    printk(KERN_INFO "micro_bench_mod loaded.");
    return 0;
}

static void __exit custom_exit(void) {
    printk(KERN_INFO "micro_bench_mod exit");
    
    proc_remove(rdmsr0_entry);
    proc_remove(rdmsr1_entry);
    proc_remove(rdmsr2_entry);
    proc_remove(rdmsr3_entry);
    proc_remove(rdmsr4_entry);
    proc_remove(rdmsr5_entry);

    proc_remove(cpuid_entry);
    proc_remove(mov_entry);
    proc_remove(nop_entry);

    proc_remove(sys_call_overhead_proc_entry);
    sysfs_remove_file(syscall_kobj, &syscall_overhead_attr.attr);
    kobject_put(syscall_kobj);
    
    // must be last
    proc_remove(micro_bench_proc_dir);
    
}

module_init(custom_init);

module_exit(custom_exit);