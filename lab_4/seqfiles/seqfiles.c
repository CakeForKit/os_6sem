#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kathrine");

#define DIRNAME "mydir"
#define FILENAME "myf"
#define SYMNAME "mys"
#define BUF_SIZE 2000
#define SIZEPIDS 3

static struct proc_dir_entry *dir;
static struct proc_dir_entry *file;
static struct proc_dir_entry *sym;

static int pids[SIZEPIDS] = {-1, -1, -1};
static int npids = 0;
static int cur_ind = 0;
static char fortune_buf[BUF_SIZE];
// static int fpid = -1;
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

static void *seq_start(struct seq_file *m, loff_t *offset) {
    printk(KERN_INFO "+ seq_start: m=%p, pos=%Ld.\n", m, *offset);
    if (cur_ind == SIZEPIDS)
        return NULL;
    return &pids[cur_ind];
}

static int seq_show(struct seq_file *m, void *v) {
    int* pv = (int *)v;
    if (pv != NULL) 
        printk(KERN_INFO "+ seq_show: m=%p, v=%p\n", m, v);
    else {
        printk(KERN_INFO "+ seq_show: m=%p, v=NULL\n", m);
        return -EINVAL;
    }
    
    int fpid = *pv;
    if (fpid == -1) {
        return 0;
    }

    struct task_struct *task = pid_task(find_vpid(fpid), PIDTYPE_PID);
    if (task == NULL) {
        seq_printf(m, "No such pid %d\n", fpid);
        return 0;
    }

    char flags[BUF_SIZE];
    parse_task_flags(task->flags, flags, BUF_SIZE);
    int len = snprintf(fortune_buf, BUF_SIZE, 
        "pid=%d\nppid=%d\n"
        "comm=%s\npcomm=%s\n"
        "state=%d\nprio=%d\npolicy=%d\n" 
        "exit_state=%d\nexit_code=%d\nexit_signal=%d\n"
        // "utime=%llu\nstime=%llu\n"
        "flags=%s\n\n",
        task->pid, task->parent->pid,
        task->comm, task->parent->comm,
        task->__state, task->prio, task->policy,
        task->exit_state, task->exit_code, task->exit_signal,
        // task->utime, task->stime,
        flags
    );
    if (len < 0) {
        return -EFAULT;
    }

    seq_printf(m, fortune_buf);
    return 0;
}


static void *seq_next(struct seq_file *m, void *v, loff_t *pos) {
    int* pv = (int *)v;
    if (pv != NULL) 
        printk(KERN_INFO "+ seq_next: m=%p, v=%p pos=%llu\n", m, v, *pos);
    else {
        printk(KERN_INFO "+ seq_next: m=%p, v=NULL\n", m);
        return NULL;
    }
    cur_ind++;
    if (cur_ind >= SIZEPIDS) 
        return NULL;
    return &pids[cur_ind];
 }

 
static void seq_stop(struct seq_file *m, void *v) {
    int* pv = (int *)v;
    if (pv != NULL) 
        printk(KERN_INFO "+ seq_stop: m=%p, v=%p\n", m, v);
    else 
        printk(KERN_INFO "+ seq_stop: m=%p, v=NULL\n", m);
}

const struct seq_operations seq_ops = {
    .start = seq_start,
    .next  = seq_next,
    .stop  = seq_stop,
    .show  = seq_show
};

static int proc_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "+ proc_open\n");
    return seq_open(file, &seq_ops);
}

static int proc_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "+ proc_release\n");
    return seq_release(inode, file);
}

static ssize_t proc_read(struct file *filp, char __user *buf, size_t count, loff_t *offset) {
    printk(KERN_INFO "+ proc_read: offset=%lld, count=%zu\n", *offset, count);
    return seq_read(filp, buf, count, offset);
}

static ssize_t proc_write(struct file *filp, 
    const char __user *buf,
    size_t count, loff_t *offset) {
    if (*offset > 0) {
        return 0;
    }

    char ubuf[PARSE_SIZE];
    if (copy_from_user(ubuf, buf, count))
        return -EFAULT;
    ubuf[count] = 0;

    if (kstrtoint(ubuf, 10, &pids[npids]))
        return -EINVAL;
    // int res = sscanf(ubuf, "%d %d %d", &pids[0], &pids[1], &pids[2]);
    // if (res != SIZEPIDS) {
    //     printk(KERN_INFO "+ proc_write: ERR res - %d \n", res);
    //     return -EINVAL;
    // }

    printk(KERN_INFO "+ proc_write: offset=%lld, count=%zu pid=%d\n", *offset, count, pids[npids]);
    npids++;
    *offset = count;
    return count;
}

static const struct proc_ops proc_fops = {
    .proc_open = proc_open,
    .proc_release = proc_release,
    .proc_read = proc_read,
    .proc_write = proc_write,
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

