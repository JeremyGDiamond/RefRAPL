// SPDX-License-Identifier: GPL-2.0-or-later
/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2000-2008 H. Peter Anvin - All Rights Reserved
 *   Copyright 2009 Intel Corporation; author: H. Peter Anvin
 *
 * ----------------------------------------------------------------------- */

/*
 * x86 MSR access device
 *
 * This device is accessed by lseek() to the appropriate register number
 * and then read/write in chunks of 8 bytes.  A larger size means multiple
 * reads or writes of the same register.
 *
 * This driver uses /dev/cpu/%d/msr where %d is the minor number, and on
 * an SMP box will direct the access to CPU %d.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/smp.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/notifier.h>
#include <linux/uaccess.h>
#include <linux/gfp.h>
#include <linux/security.h>

#include <asm/cpufeature.h>
#include <asm/msr.h>

#include <linux/hashtable.h>
#include <linux/jhash.h>
#include <linux/ktime.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define TIME_TABLE_BITS 10

struct time_key{
  u32 register_num;
  u32 cpu_num;
};

struct time_entry{
  struct time_key key;
  u32 access_count;
  u64 micro_total;
  struct hlist_node hnode;
};

DEFINE_HASHTABLE(time_table, TIME_TABLE_BITS);
static DEFINE_SPINLOCK(timetable_lock);

u32 hash_time_key(const struct time_key *key) {
    return jhash(key, sizeof(struct time_key), 0);
}

struct time_entry *find_time(struct time_key *key) {
    struct time_entry *entry;
    u32 hash = hash_time_key(key);
	unsigned long lock_flags = 0;

	spin_lock_irqsave(&timetable_lock, lock_flags);

    hash_for_each_possible(time_table, entry, hnode, hash) {
        if (memcmp(&entry->key, key, sizeof(struct time_key)) == 0){
    			spin_unlock_irqrestore(&timetable_lock, lock_flags);
		    	return entry;
	  	}
    }

	spin_unlock_irqrestore(&timetable_lock, lock_flags);
    return NULL;
}

u8 add_time(struct time_key *key, u64 micro_dur) {
    struct time_entry *entry = kmalloc(sizeof(*entry), GFP_KERNEL);
    u8 ret = 0;
	  unsigned long lock_flags = 0;

	  spin_lock_irqsave(&timetable_lock, lock_flags);

	  if (!entry){
		  spin_unlock_irqrestore(&timetable_lock, lock_flags); 
		  return 0;
	  }

    entry->key = *key;
    entry->access_count = 1;
    entry->micro_total = micro_dur;

    hash_add(time_table, &entry->hnode, hash_time_key(key));
	  spin_unlock_irqrestore(&timetable_lock, lock_flags);
	  return ret;
}

static void timer_update(u32 cpu_num, u32 register_num, u64 micro_dur){
	struct time_key *key;
	struct time_entry *entry;
	unsigned long lock_flags;

	key = kmalloc(sizeof(struct time_key), GFP_KERNEL);
	if (!key)
		return;

  key->cpu_num = cpu_num;
  key->register_num = register_num;

	// lookup in table
	entry = find_time(key);

	// if in table add to count, update ts and think about reclass
	if(entry){
		spin_lock_irqsave(&timetable_lock, lock_flags);
		entry->access_count++;
		entry->micro_total += micro_dur;
		
		spin_unlock_irqrestore(&timetable_lock, lock_flags);
		//free dupe key
		kfree(key);
	}

	// else add new to table 
	else{
		add_time(key, micro_dur);
	}
	return;
}

static enum cpuhp_state cpuhp_msr_state;

enum allow_write_msrs {
	MSR_WRITES_ON,
	MSR_WRITES_OFF,
	MSR_WRITES_DEFAULT,
};

static enum allow_write_msrs allow_writes = MSR_WRITES_DEFAULT;

static ssize_t msr_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
  u64 start = 0;
  u64 end = 0;
  u32 __user *tmp = (u32 __user *) buf;
	u32 data[2];
	u32 reg = *ppos;
	int cpu = iminor(file_inode(file));
	int err = 0;
	ssize_t bytes = 0;
  
  start = ktime_to_ns(ktime_get());

	if (count % 8)
		return -EINVAL;	/* Invalid chunk size */

	for (; count; count -= 8) {
		err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
		if (err)
			break;
		if (copy_to_user(tmp, &data, 8)) {
			err = -EFAULT;
			break;
		}
		tmp += 2;
		bytes += 8;
	}
  end = ktime_to_ns(ktime_get());
  timer_update(cpu, reg, (end-start)/1000);

	return bytes ? bytes : err;
}

static int filter_write(u32 reg)
{
	/*
	 * MSRs writes usually happen all at once, and can easily saturate kmsg.
	 * Only allow one message every 30 seconds.
	 *
	 * It's possible to be smarter here and do it (for example) per-MSR, but
	 * it would certainly be more complex, and this is enough at least to
	 * avoid saturating the ring buffer.
	 */
	static DEFINE_RATELIMIT_STATE(fw_rs, 30 * HZ, 1);

	switch (allow_writes) {
	case MSR_WRITES_ON:  return 0;
	case MSR_WRITES_OFF: return -EPERM;
	default: break;
	}

	if (!__ratelimit(&fw_rs))
		return 0;

	pr_warn("Write to unrecognized MSR 0x%x by %s (pid: %d).\n",
	        reg, current->comm, current->pid);
	pr_warn("See https://git.kernel.org/pub/scm/linux/kernel/git/tip/tip.git/about for details.\n");

	return 0;
}

static ssize_t msr_write(struct file *file, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	const u32 __user *tmp = (const u32 __user *)buf;
	u32 data[2];
	u32 reg = *ppos;
	int cpu = iminor(file_inode(file));
	int err = 0;
	ssize_t bytes = 0;

	err = security_locked_down(LOCKDOWN_MSR);
	if (err)
		return err;

	err = filter_write(reg);
	if (err)
		return err;

	if (count % 8)
		return -EINVAL;	/* Invalid chunk size */

	for (; count; count -= 8) {
		if (copy_from_user(&data, tmp, 8)) {
			err = -EFAULT;
			break;
		}

		add_taint(TAINT_CPU_OUT_OF_SPEC, LOCKDEP_STILL_OK);

		err = wrmsr_safe_on_cpu(cpu, reg, data[0], data[1]);
		if (err)
			break;

		tmp += 2;
		bytes += 8;
	}

	return bytes ? bytes : err;
}

static long msr_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{
	u32 __user *uregs = (u32 __user *)arg;
	u32 regs[8];
	int cpu = iminor(file_inode(file));
	int err;

	switch (ioc) {
	case X86_IOC_RDMSR_REGS:
		if (!(file->f_mode & FMODE_READ)) {
			err = -EBADF;
			break;
		}
		if (copy_from_user(&regs, uregs, sizeof(regs))) {
			err = -EFAULT;
			break;
		}
		err = rdmsr_safe_regs_on_cpu(cpu, regs);
		if (err)
			break;
		if (copy_to_user(uregs, &regs, sizeof(regs)))
			err = -EFAULT;
		break;

	case X86_IOC_WRMSR_REGS:
		if (!(file->f_mode & FMODE_WRITE)) {
			err = -EBADF;
			break;
		}
		if (copy_from_user(&regs, uregs, sizeof(regs))) {
			err = -EFAULT;
			break;
		}
		err = security_locked_down(LOCKDOWN_MSR);
		if (err)
			break;

		err = filter_write(regs[1]);
		if (err)
			return err;

		add_taint(TAINT_CPU_OUT_OF_SPEC, LOCKDEP_STILL_OK);

		err = wrmsr_safe_regs_on_cpu(cpu, regs);
		if (err)
			break;
		if (copy_to_user(uregs, &regs, sizeof(regs)))
			err = -EFAULT;
		break;

	default:
		err = -ENOTTY;
		break;
	}

	return err;
}

static int msr_open(struct inode *inode, struct file *file)
{
	unsigned int cpu = iminor(file_inode(file));
	struct cpuinfo_x86 *c;

	if (!capable(CAP_SYS_RAWIO))
		return -EPERM;

	if (cpu >= nr_cpu_ids || !cpu_online(cpu))
		return -ENXIO;	/* No such CPU */

	c = &cpu_data(cpu);
	if (!cpu_has(c, X86_FEATURE_MSR))
		return -EIO;	/* MSR not supported */

	return 0;
}

/*
 * File operations we support
 */
static const struct file_operations msr_fops = {
	.owner = THIS_MODULE,
	.llseek = no_seek_end_llseek,
	.read = msr_read,
	.write = msr_write,
	.open = msr_open,
	.unlocked_ioctl = msr_ioctl,
	.compat_ioctl = msr_ioctl,
};

static char *msr_devnode(const struct device *dev, umode_t *mode)
{
	return kasprintf(GFP_KERNEL, "cpu/%u/msr", MINOR(dev->devt));
}

static const struct class msr_class = {
	.name		= "msr",
	.devnode	= msr_devnode,
};

static int msr_device_create(unsigned int cpu)
{
	struct device *dev;

	dev = device_create(&msr_class, NULL, MKDEV(MSR_MAJOR, cpu), NULL,
			    "msr%d", cpu);
	return PTR_ERR_OR_ZERO(dev);
}

static int msr_device_destroy(unsigned int cpu)
{
	device_destroy(&msr_class, MKDEV(MSR_MAJOR, cpu));
	return 0;
}

static struct proc_dir_entry* proc_entry;

static int time_table_show(struct seq_file *m, void *v)
{
    int bkt, first = 1;
    struct time_entry *entry;

    seq_puts(m, "[\n");

    hash_for_each(time_table, bkt, entry, hnode) {
        if (!first)
            seq_puts(m, ",\n");
        else
            first = 0;

	      printk(KERN_INFO "cpu_num: %u, register_num: %u", entry->key.cpu_num, entry->key.register_num);

        seq_printf(m,
            "  {\n"
            "    \"cpu_num\": \"%u\",\n"
            "    \"register_num\": %x,\n"
            "    \"access_count\": \"%u\",\n"
            "    \"micro_total\": %llu,\n"
            "  }\n",
            entry->key.cpu_num, entry->key.register_num,
            entry->access_count, entry->micro_total);
    }

    seq_puts(m, "\n]\n");
    return 0;
}

static int time_table_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, time_table_show, NULL);
}

static struct proc_ops pops =
{
	  .proc_open    = time_table_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static int __init msr_init(void)
{
	int err;

	if (__register_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr", &msr_fops)) {
		pr_err("unable to get major %d for msr\n", MSR_MAJOR);
		return -EBUSY;
	}
	err = class_register(&msr_class);
	if (err)
		goto out_chrdev;

	err  = cpuhp_setup_state(CPUHP_AP_ONLINE_DYN, "x86/msr:online",
				 msr_device_create, msr_device_destroy);
	if (err < 0)
		goto out_class;
	cpuhp_msr_state = err;
  // proc entry for timer table
  proc_entry = proc_create("timer_msr", 0444, NULL, &pops);
	return 0;

out_class:
	class_unregister(&msr_class);
out_chrdev:
	__unregister_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr");
	return err;
}
module_init(msr_init);

static void __exit msr_exit(void)
{
	cpuhp_remove_state(cpuhp_msr_state);
	class_unregister(&msr_class);
  proc_remove(proc_entry);
	__unregister_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr");
}
module_exit(msr_exit)

static int set_allow_writes(const char *val, const struct kernel_param *cp)
{
	/* val is NUL-terminated, see kernfs_fop_write() */
	char *s = strstrip((char *)val);

	if (!strcmp(s, "on"))
		allow_writes = MSR_WRITES_ON;
	else if (!strcmp(s, "off"))
		allow_writes = MSR_WRITES_OFF;
	else
		allow_writes = MSR_WRITES_DEFAULT;

	return 0;
}

static int get_allow_writes(char *buf, const struct kernel_param *kp)
{
	const char *res;

	switch (allow_writes) {
	case MSR_WRITES_ON:  res = "on"; break;
	case MSR_WRITES_OFF: res = "off"; break;
	default: res = "default"; break;
	}

	return sprintf(buf, "%s\n", res);
}

static const struct kernel_param_ops allow_writes_ops = {
	.set = set_allow_writes,
	.get = get_allow_writes
};

module_param_cb(allow_writes, &allow_writes_ops, NULL, 0600);

MODULE_AUTHOR("H. Peter Anvin <hpa@zytor.com>");
MODULE_AUTHOR("Jeremy Diamond");
MODULE_DESCRIPTION("x86 generic MSR driver");
MODULE_LICENSE("GPL");
