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
MODULE_DESCRIPTION("Process information viewer");
MODULE_VERSION("0.5");

#define DIRNAME   "seqdir"
#define FILENAME  "seqfile"
#define SYMLINK   "seqlink"
#define FILEPATH  DIRNAME "/" FILENAME
#define PID_BUF_SIZE   64
#define MAX_PIDS       3

static struct proc_dir_entry *fortune_dir;
static struct proc_dir_entry *fortune_file;
static struct proc_dir_entry *fortune_link;
static int stored_pids[MAX_PIDS] = { -1, -1, -1 };
static int num_pids = 0;

struct iter_state
{
    loff_t pos;my_stopь
    struct task_struct *tasks[MAX_PIDS];
    int current_pid;
};

static void *my_start(struct seq_file *m, loff_t *pos)
{
    struct iter_state *state;
    int i;
ь
    printk(KERN_INFO "++++ START: m=%p, v=%p, pos=%lld\n",
           m, pos, *pos);

    if (num_pids == 0 || *pos >= num_pids)
    {
        return NULL;
    }

    state = kmalloc(sizeof(struct iter_state), GFP_KERNEL);
    if (!state)
    {
        return NULL;
    }

    state->pos = *pos;
    state->current_pid = 0;

    for (i = 0; i < num_pids; i++)
    {
        if (stored_pids[i] <= 0)
        {
            state->tasks[i] = NULL;
            continue;
        }

        struct pid *pid_struct = find_get_pid(stored_pids[i]);
        state->tasks[i] = pid_task(pid_struct, PIDTYPE_PID);
        put_pid(pid_struct);

        if (!state->tasks[i])
        {
            printk(KERN_INFO "Failed to find task for PID %d\n", stored_pids[i]);
        }
    }

    return state;
}

static void *my_next(struct seq_file *m, void *v, loff_t *pos)
{
    struct iter_state *state = v;

    printk(KERN_INFO "++++ NEXT: m=%p, v=%p, pos_ptr=%p, pos=%lld\n", 
        m, v, pos, *pos);

    (*pos)++;
    state->current_pid++;
    
    if (*pos >= num_pids)
    {
        kfree(state);
        return NULL;
    }

    return state;
}

static void my_stop(struct seq_file *m, void *v)
{
    printk(KERN_INFO "++++ STOP: m=%p, v=%p\n", m, v);

    if (v)
    {
        kfree(v);
    }
}

static int my_show(struct seq_file *m, void *v)
{
    struct iter_state *state = v;
    struct task_struct *task;
    int current_pid = state->current_pid;

    printk(KERN_INFO "++++ SHOW: m=%p, v=%p, pos=%lld, current_pid=%d\n",
           m, v, state->pos, current_pid);

    if (current_pid >= num_pids || !state->tasks[current_pid])
    {
        return 0;
    }

    task = state->tasks[current_pid];
    seq_printf(m, "=== PID %d ===\n", stored_pids[current_pid]);
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

    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf,
                      size_t count, loff_t *ppos)
{
    printk(KERN_INFO "++++ READ");
    return seq_read(file, buf, count, ppos);
}

static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "++++ RELEASE");
    return seq_release(inode, file);
}

static ssize_t my_write(struct file *file,
    const char __user *ubuf,
    size_t len, loff_t *ppos)
{
    printk(KERN_INFO "++++ WRITE");
    char buf[PID_BUF_SIZE];
    int pids[3] = {0};
    int count = 0;

    if (len == 0 || len >= PID_BUF_SIZE)
    {
        return -EINVAL;
    }
    if (copy_from_user(buf, ubuf, len))
    {
        return -EFAULT;
    }
    buf[len] = '\0';

    count = sscanf(buf, "%d %d %d", &stored_pids[0], &stored_pids[1], &stored_pids[2]);
    if (count != MAX_PIDS)
    {
        return -EINVAL;
    }
    num_pids = count;

    *ppos = len;
    return len;
}


static const struct seq_operations seq_ops =
{
    .start = my_start,
    .next  = my_next,
    .stop  = my_stop,
    .show  = my_show
};

static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "++++ OPEN");
    return seq_open(file, &seq_ops);
}

static const struct proc_ops fops =
{
    .proc_open    = my_open,
    .proc_read    = my_read,
    .proc_write   = my_write,
    .proc_release = my_release
};

static int __init fortune_init(void)
{
    printk(KERN_INFO "++++ INIT");
    fortune_dir = proc_mkdir(DIRNAME, NULL);
    if (!fortune_dir)
    {
        return -ENOMEM;
    }

    fortune_file = proc_create(FILENAME, 0666, fortune_dir, &fops);
    if (!fortune_file)
    {
        remove_proc_entry(DIRNAME, NULL);
        return -ENOMEM;
    }

    fortune_link = proc_symlink(SYMLINK, NULL, FILEPATH);
    if (!fortune_link)
    {
        remove_proc_entry(FILENAME, fortune_dir);
        remove_proc_entry(DIRNAME, NULL);
        return -ENOMEM;
    }

    return 0;
}

static void __exit fortune_exit(void)
{
    if (fortune_link)
        remove_proc_entry(SYMLINK, NULL);
    if (fortune_file)
        remove_proc_entry(FILENAME, fortune_dir);
    if (fortune_dir)
        remove_proc_entry(DIRNAME, NULL);
}

module_init(fortune_init);
module_exit(fortune_exit);