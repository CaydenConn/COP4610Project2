#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("COP4610");
MODULE_DESCRIPTION("Timer kernel module that tracks elapsed time via /proc/timer");

#define PROC_NAME "timer"
#define BUF_SIZE 256

static struct proc_dir_entry *proc_entry;
static struct timespec64 last_time;
static int has_been_read = 0;  /* flag: 1 after the first read */

static ssize_t timer_proc_read(struct file *file, char __user *ubuf,
                               size_t count, loff_t *ppos)
{
    struct timespec64 now;
    char buf[BUF_SIZE];
    int len = 0;

    if (*ppos > 0)
        return 0;

    ktime_get_real_ts64(&now);

    len += snprintf(buf + len, BUF_SIZE - len,
                    "current time: %lld.%09ld\n",
                    (long long)now.tv_sec, now.tv_nsec);

    if (has_been_read) {
        long long elapsed_sec;
        long elapsed_nsec;

        elapsed_sec  = now.tv_sec  - last_time.tv_sec;
        elapsed_nsec = now.tv_nsec - last_time.tv_nsec;

        if (elapsed_nsec < 0) {
            elapsed_sec  -= 1;
            elapsed_nsec += 1000000000L;
        }

        len += snprintf(buf + len, BUF_SIZE - len,
                        "elapsed time: %lld.%09ld\n",
                        elapsed_sec, elapsed_nsec);
    }

    last_time = now;
    has_been_read = 1;

    if (copy_to_user(ubuf, buf, len))
        return -EFAULT;

    *ppos = len;
    return len;
}

static const struct proc_ops timer_proc_ops = {
    .proc_read = timer_proc_read,
};

static int __init timer_init(void)
{
    proc_entry = proc_create(PROC_NAME, 0644, NULL, &timer_proc_ops);
    if (!proc_entry) {
        printk(KERN_ERR "my_timer: failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    ktime_get_real_ts64(&last_time);
    has_been_read = 0;

    printk(KERN_INFO "my_timer: module loaded, /proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit timer_exit(void)
{
    proc_remove(proc_entry);
    printk(KERN_INFO "my_timer: module unloaded, /proc/%s removed\n", PROC_NAME);
}

module_init(timer_init);
module_exit(timer_exit);
