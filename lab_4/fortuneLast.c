#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kathrine");

#define DIRNAME "mydir"
#define FILENAME "myf"
#define SYMNAME "mys"
#define BUF_SIZE 1024

static struct proc_dir_entry *dir;
static struct proc_dir_entry *file;
static struct proc_dir_entry *sym;

static char fortune_buf[BUF_SIZE];
static int fpid = -1;

#define PARSE_SIZE 16

static void parse_task_flags(unsigned int flags, char *buf, size_t buf_size)
{
    size_t offset = 0;
    
    offset += snprintf(buf + offset, buf_size - offset, "(");
    
    if (flags & PF_IDLE) offset += snprintf(buf + offset, buf_size - offset, "IDLE,");
    if (flags & PF_EXITING) offset += snprintf(buf + offset, buf_size - offset, "EXITING,");
    if (flags & PF_KTHREAD) offset += snprintf(buf + offset, buf_size - offset, "KTHREAD,");
    if (flags & PF_RANDOMIZE) offset += snprintf(buf + offset, buf_size - offset, "RANDOMIZE,");
    if (flags & PF_KSWAPD) offset += snprintf(buf + offset, buf_size - offset, "KSWAPD,");
    if (flags & PF_VCPU) offset += snprintf(buf + offset, buf_size - offset, "VCPU,");
    if (flags & PF_WQ_WORKER) offset += snprintf(buf + offset, buf_size - offset, "WQ_WORKER,");
    if (flags & PF_FORKNOEXEC) offset += snprintf(buf + offset, buf_size - offset, "FORKNOEXEC,");
    if (flags & PF_MCE_PROCESS) offset += snprintf(buf + offset, buf_size - offset, "MCE_PROCESS,");
    if (flags & PF_SUPERPRIV) offset += snprintf(buf + offset, buf_size - offset, "SUPERPRIV,");
    if (flags & PF_DUMPCORE) offset += snprintf(buf + offset, buf_size - offset, "DUMPCORE,");
    if (flags & PF_SIGNALED) offset += snprintf(buf + offset, buf_size - offset, "SIGNALED,");
    if (flags & PF_MEMALLOC) offset += snprintf(buf + offset, buf_size - offset, "MEMALLOC,");
    if (flags & PF_NPROC_EXCEEDED) offset += snprintf(buf + offset, buf_size - offset, "NPROC_EXCEEDED,");
    if (flags & PF_USED_MATH) offset += snprintf(buf + offset, buf_size - offset, "USED_MATH,");
    if (flags & PF_NOFREEZE) offset += snprintf(buf + offset, buf_size - offset, "NOFREEZE,");
    if (flags & PF_KSWAPD) offset += snprintf(buf + offset, buf_size - offset, "KSWAPD,");
    if (flags & PF_MEMALLOC_NOIO) offset += snprintf(buf + offset, buf_size - offset, "MEMALLOC_NOIO,");
    if (flags & PF_NO_SETAFFINITY) offset += snprintf(buf + offset, buf_size - offset, "PF_NO_SETAFFINITY,");
    
    offset += snprintf(buf + offset, buf_size - offset, ")");
    if (offset > strlen("(")) {
        buf[offset-1] = '\0'; 
    }
}

static ssize_t fortune_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
    printk(KERN_INFO " + read: pos=%lld, count=%zu\n", *f_pos, count);
    struct task_struct *task;
    int len;
    if (*f_pos > 0 || fpid == -1) {
        return 0;
    }
    if (count >= BUF_SIZE) {
        count = BUF_SIZE -1;
    }

    task = pid_task(find_vpid(fpid), PIDTYPE_PID);
    if (task == NULL) {
        len = sprintf(fortune_buf, "No such pid %d", fpid);
        if (copy_to_user(buf, fortune_buf, len))
            return -EFAULT;
        *f_pos += len;
        return len;
    }

    char flags[BUF_SIZE];
    parse_task_flags(task->flags, flags, BUF_SIZE);
    len = snprintf(fortune_buf, BUF_SIZE, 
        "pid=%d\n"
        "ppid=%d\n"
        "comm=%s\n"
        "pcomm=%s\n"
        "state=%d\n"
        "prio=%d\n"
        // "normal_prio=%d\n"
        // "static_prio=%d\n"
        // "rt_prio=%d\n"
        "policy=%d\n" 
        "exit_state=%d\n"
        "exit_code=%d\n" 
        "exit_signal=%d\n"
        "utime=%llu\n"
        "stime=%llu\n"
        "flags=%s\n",
        task->pid,
        task->parent->pid,
        task->comm,
        task->parent->comm,
        task->__state,
        task->prio,
        // task->normal_prio,
        // task->static_prio,
        // task->rt_priority,
        task->policy,
        task->exit_state,
        task->exit_code,
        task->exit_signal,
        task->utime,
        task->stime,
        flags
    );

    if (copy_to_user(buf, fortune_buf, len))
        return -EFAULT;
    *f_pos += len;
    return len;
}

static ssize_t fortune_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    printk(KERN_INFO " + write: pos=%lld, count=%zu\n", *f_pos, count);
    if (*f_pos > 0) {
        return 0;
    }

    char ubuf[PARSE_SIZE];
    if (copy_from_user(ubuf, buf, count))
        return -EFAULT;

    if (kstrtoint(ubuf, 10, &fpid))
        return -EINVAL;

    return count;
}

static int proc_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "+ open\n");
    return 0;
}

static int proc_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "+ release\n");
    return 0;
}

static const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_release = proc_release,
    .proc_read = fortune_read,
    .proc_write = fortune_write,
};

static int __init fortune_init(void) {
    printk("+ init\n");
    dir = proc_mkdir(DIRNAME, NULL);
    if (dir == NULL) {
        printk(KERN_ERR "+ proc_mkdir failed\n");
        return -ENOMEM;
    }
    file = proc_create(FILENAME, 0666, dir, &proc_fops);
    if (file == NULL) {
        printk(KERN_ERR "+ proc_create failed\n");
        proc_remove(file);
        return -ENOMEM;
    }
    sym = proc_symlink(SYMNAME, NULL, DIRNAME "/" FILENAME);
    if (sym == NULL) {
        printk(KERN_ERR "+ proc_create failed\n");
        proc_remove(file);
        proc_remove(dir);
        return -ENOMEM;
    }
    return 0;
}

static void __exit fortune_exit(void) {
    printk("+ exit\n");
    proc_remove(sym);
    proc_remove(file);
    proc_remove(dir);
}

module_init(fortune_init);
module_exit(fortune_exit);

