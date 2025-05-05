#include <linux/init_task.h>
#include <linux/module.h>
// #include <linux/sched.h>
#include <linux/proc_fs.h>
// #include <linux/fs_struct.h>
#include <linux/pid.h>
// #include <linux/fs.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kathrine");

#define PROC_DIR_NAME "my_dir"
#define PROC_FILE_NAME "my_file"
#define PROC_SYMLINK_NAME "my_symlink"

// static struct proc_dir_entry *my_proc_dir;
static struct proc_dir_entry *my_proc_file;
// static struct proc_dir_entry *my_proc_symlink;

// Глобальная переменная для хранения PID
static pid_t current_pid = 0;
static DEFINE_MUTEX(pid_mutex); // Мьютекс для защиты доступа к current_pid

static ssize_t procfs_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos) {
    printk(KERN_INFO " + procfs_read: ppos=%lld, count=%d\n", *ppos, count);
    struct task_struct *task;
    char output_buf[256];
    int len;
    pid_t local_pid;

    mutex_lock(&pid_mutex);
    local_pid = current_pid;
    mutex_unlock(&pid_mutex);

    if (*ppos > 0 || local_pid == 0)
        return 0;

    task = pid_task(find_vpid(local_pid), PIDTYPE_PID);
    if (!task)
        return sprintf(output_buf, "Процесс с PID %d не найден.\n", local_pid);
    
    printk(KERN_INFO "+ Запрошен PID: %d, найден процесс: %s (%d)\n", 
       local_pid, task ? task->comm : "NULL", task ? task->pid : -1);

    len = snprintf(output_buf, sizeof(output_buf),
                  "PID: %d\nИмя: %s\nСостояние: %#x\nРодительский PID: %d\n",
                  task->pid, task->comm, task->__state, task->real_parent->pid);

    if (*ppos >= len)
        return 0;

    if (copy_to_user(buffer, output_buf, len))
        return -EFAULT;

    *ppos += len;
    return len;
}

static ssize_t procfs_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset) {
    printk(KERN_INFO " + procfs_write\n");
    char buf[30];
    pid_t new_pid;
    int ret;

    if (count >= sizeof(buf))
        return -EINVAL;

    if (copy_from_user(buf, buffer, count))
        return -EFAULT;

    buf[count] = '\0';
    ret = kstrtoint(buf, 10, &new_pid);
    if (ret)
        return ret;

    if (new_pid < 1)
        return -EINVAL;

    mutex_lock(&pid_mutex);
    current_pid = new_pid;
    mutex_unlock(&pid_mutex);

    return count;
}


static int procfs_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO " + procfs_open\n");
    return 0;
}

static int procfs_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO " + procfs_release\n");
    return 0;
}


static const struct proc_ops fops =
{
    .proc_open = procfs_open,
    .proc_read = procfs_read,
    .proc_write = procfs_write,
    .proc_release = procfs_release
};

static int __init mod_init(void)
{
    printk(KERN_INFO " + module is loaded.\n");

    
    my_proc_file = proc_create(PROC_FILE_NAME, 0666, NULL, &fops);
    if (!my_proc_file) {
        printk(KERN_INFO " + Error: proc_create\n");
        // pr_err("Failed to create /proc/%s/%s\n", PROC_DIR_NAME, PROC_FILE_NAME);
        // remove_proc_entry(PROC_DIR_NAME, NULL);
        return -ENOMEM;
    }
//     struct task_struct *task = &init_task;
//     do
//     {
//         printk(KERN_INFO " + %s (%d) (%d - state, %d - prio, flags - %d, policy - %d), parent %s (%d), d_name %s",
//             task->comm, task->pid, task->__state, task->prio, task->flags, task->policy, task->parent->comm, task->parent->pid, task->fs->root.dentry->d_name.name);
//     } while ((task = next_task(task)) != &init_task);

//    // task = current;
//     printk(KERN_INFO " + %s (%d) (%d - state, %d - prio, flags - %d, policy - %d), parent %s (%d), d_name %s",
//         current->comm, current->pid, current->__state, current->prio, current->flags, current->policy, current->parent->comm, current->parent->pid, current->fs->root.dentry->d_name.name);
    
    return 0;
}

static void __exit mod_exit(void)
{
    printk(KERN_INFO " + module is unloaded.\n");
    remove_proc_entry(PROC_FILE_NAME, NULL);
}

module_init(mod_init);
module_exit(mod_exit);