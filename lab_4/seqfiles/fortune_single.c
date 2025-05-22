#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TEST");

#define DIRNAME   "seqdir"
#define FILENAME  "seqfile"
#define SYMLINK   "seqlink"
#define FILEPATH  DIRNAME "/" FILENAME
#define PID_BUF_SIZE   64
#define MAX_PIDS       3

static struct proc_dir_entry *dir;
static struct proc_dir_entry *file;
static struct proc_dir_entry *link;
static pid_t stored_pids[MAX_PIDS] = { -1, -1, -1 };
static int num_pids = 0;

struct pid_info {
    struct task_struct *task;
    pid_t pid;
    int policy_num;
    const char *policy_name;
};

static const char *get_sched_policy(struct task_struct *task, int *policy_num)
{
    *policy_num = task->policy;
    switch (*policy_num)
    {
        case SCHED_NORMAL: return "SCHED_NORMAL/OTHER";
        case SCHED_FIFO: return "SCHED_FIFO";
        case SCHED_RR: return "SCHED_RR";
        case SCHED_BATCH: return "SCHED_BATCH";
        case SCHED_IDLE: return "SCHED_IDLE";
        case SCHED_DEADLINE: return "SCHED_DEADLINE";
        default: return "UNKNOWN";
    }
}

static int my_seq_show(struct seq_file *m, void *v)
{
    struct pid_info pids_info[MAX_PIDS];
    int i, valid_pids = 0;

    printk(KERN_INFO "---- SHOW: m=%p, v=%p\n", m, v);

    // Collect information for all valid PIDs
    for (i = 0; i < num_pids; i++) {
        if (stored_pids[i] <= 0)
            continue;

        struct pid *pid_struct = find_get_pid(stored_pids[i]);
        pids_info[valid_pids].task = pid_task(pid_struct, PIDTYPE_PID);
        put_pid(pid_struct);

        if (pids_info[valid_pids].task) {
            pids_info[valid_pids].pid = stored_pids[i];
            pids_info[valid_pids].policy_name = get_sched_policy(pids_info[valid_pids].task, 
                                               &pids_info[valid_pids].policy_num);
            valid_pids++;
        }
    }

    if (valid_pids == 0) {
        seq_puts(m, "No valid processes to display\n");
        return 0;
    }

    // Display information for each valid process
    for (i = 0; i < valid_pids; i++) {
        struct task_struct *task = pids_info[i].task;
        
        seq_printf(m, "=== PID %d ===\n", pids_info[i].pid);
        seq_printf(m, "PID: %d\n", task->pid);
        seq_printf(m, "COMM: %s\n", task->comm);
        seq_printf(m, "PPID: %d\n", task->real_parent->pid);
        seq_printf(m, "TGID: %d\n", task->tgid);
        seq_printf(m, "STATE: %ld\n", task->__state);
        seq_printf(m, "FLAGS: 0x%x\n", task->flags);
        seq_printf(m, "POLICY: %d\n", task->policy);
        seq_printf(m, "PRIO: %d\n", task->prio);
        seq_printf(m, "NICE: %d\n", task_nice(task));
        seq_printf(m, "NUM_THREADS: %d\n", task->signal->nr_threads);
        seq_printf(m, "TOTAL_VM: %lu\n", task->mm ? task->mm->total_vm : 0UL);
        seq_printf(m, "START_TIME: %llu\n", task->start_time);
        seq_printf(m, "\n");
    }

    return 0;
}

static int my_seq_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "---- OPEN\n");
    return single_open(file, my_seq_show, NULL);
}

static ssize_t my_seq_write(struct file *file,
                          const char __user *buf,
                          size_t len, loff_t *ppos)
{
    char pid_buf[PID_BUF_SIZE];
    int i, count;

    printk(KERN_INFO "---- WRITE: len=%zu\n", len);

    if (len == 0 || len >= PID_BUF_SIZE)
    {
        printk(KERN_INFO "---- invalid length %zu\n", len);
        return -EINVAL;
    }

    if (copy_from_user(pid_buf, buf, len))
    {
        printk(KERN_INFO "---- copy_from_user failed\n");
        return -EFAULT;
    }
    pid_buf[len] = '\0';

    for (i = 0; i < MAX_PIDS; i++)
        stored_pids[i] = -1;

    count = sscanf(pid_buf, "%d %d %d", &stored_pids[0], &stored_pids[1], &stored_pids[2]);
    
    num_pids = 0;
    for (i = 0; i < MAX_PIDS; i++)
    {
        if (stored_pids[i] > 0)
            num_pids++;
    }

    if (num_pids == 0)
    {
        printk(KERN_INFO "---- no valid PIDs provided\n");
        return -EINVAL;
    }

    *ppos = len;
    return len;
}

static ssize_t my_seq_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
    printk(KERN_INFO "---- READ: pos=%lld\n", *ppos);
    return seq_read(file, buf, len, ppos);
}

static int my_seq_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "---- RELEASE\n");
    return single_release(inode, file);
}

static const struct proc_ops my_seq_fops =
{
    .proc_open    = my_seq_open,
    .proc_read    = my_seq_read,
    .proc_write   = my_seq_write,
    .proc_release = my_seq_release,
};

static void my_cleanup(void)
{
    if (link) remove_proc_entry(SYMLINK, NULL);
    if (file) remove_proc_entry(FILENAME, dir);
    if (dir) remove_proc_entry(DIRNAME, NULL);
}

static int __init my_seq_init(void)
{
    printk(KERN_INFO "---- INIT\n");

    dir = proc_mkdir(DIRNAME, NULL);
    if (!dir)
    {
        printk(KERN_INFO "---- proc_mkdir failed\n");
        return -ENOMEM;
    }

    file = proc_create(FILENAME, 0666, dir, &my_seq_fops);
    if (!file)
    {
        my_cleanup();
        printk(KERN_INFO "---- proc_create failed\n");
        return -ENOMEM;
    }

    link = proc_symlink(SYMLINK, NULL, FILEPATH);
    if (!link)
    {
        my_cleanup();
        printk(KERN_INFO "---- proc_symlink failed\n");
        return -ENOMEM;
    }
    return 0;
}

static void __exit my_seq_exit(void)
{
    my_cleanup();
    printk(KERN_INFO "---- EXIT\n");
}

module_init(my_seq_init);
module_exit(my_seq_exit);