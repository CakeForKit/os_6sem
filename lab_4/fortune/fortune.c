#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/pid.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kathrine");

static struct proc_dir_entry *pfile;
static struct proc_dir_entry *pdir;
static struct proc_dir_entry *pslink;

#define myfilename "myfile"
#define mydir  "mydir"
#define myslink  "mysymlink"
#define myfpath mydir "/" myfilename

static char *buf = NULL;
char tmpBuf[PAGE_SIZE];

static int readInd = 0;
static int writeInd = 0;

static void freeMemory(void)
{
    if (pslink != NULL)
        remove_proc_entry(myslink, NULL);

    if (pfile != NULL)
        remove_proc_entry(myfilename, pdir);

    if (pdir != NULL)
        remove_proc_entry(mydir, NULL);

    vfree(buf);
}

static int my_open(struct inode *spInode, struct file *spFile)
{
    printk(KERN_INFO "+ open called\n");
    return 0;
}

static int my_release(struct inode *spInode, struct file *spFile)
{
    printk(KERN_INFO "+ release called\n");
    return 0;
}

static ssize_t my_write(struct file *file, const char __user *buf,
                            size_t len, loff_t *fPos)
{
    char kbuf[16];
    pid_t pid;
    struct task_struct *task=&init_task;
    int copied_len = 0;

    printk(KERN_INFO "+write called\n");

    if (len >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    kbuf[len] = '\0'; // обязательно завершаем строку нулём

    if (kstrtoint(kbuf, 10, &pid))
        return -EINVAL;

    int found = 0;
    printk(KERN_INFO "+received pid = %d\n", pid);
    do {
        if (task->pid == pid) 
        {
            found = 1;
        	printk(KERN_INFO "+find task->pid: %d\n", task->pid);
            copied_len = snprintf(buf, PAGE_SIZE,
                "comm: %s\n"
                "pid: %d\n"
                "ppid: %d\n"
                "parent_comm: %s\n"
                "prio: %d\n"
                "policy: %d\n"
                "flags: %lx\n",
                task->comm,
                task->pid,
                task->parent->pid, 
                task->parent->comm,
                task->prio,
                task->policy,
                task->flags);
        }
    } while ((task = next_task(task)) != &init_task);
    if (task == NULL) {
        printk(KERN_INFO "+ no such pid %d\n", pid);
        return -ESRCH;
    } else if (found == 0) {
        printk(KERN_INFO "+ task not found\n");
        return len;
    }


    // Обновляем индексы для чтения и записи
    writeInd = copied_len;
    readInd = 0;

    // Освобождаем ссылку на процесс
    //put_task_struct(task);

    return len;
}

static ssize_t my_read(struct file *file, char __user *buf,
                           size_t len, loff_t *fPos)
{
    int readLen;

    printk(KERN_INFO "+ my_read\n");

    if ((*fPos > 0) || (writeInd == 0))
        return 0;

    readLen = snprintf(tmpBuf, PAGE_SIZE, "%s\n", &buf[readInd]);

    if (copy_to_user(buf, tmpBuf, readLen) != 0) {
        printk(KERN_INFO "+copy_to_user error\n");
        return -EFAULT;
    }
    *fPos += readLen;
    readInd += readLen;

    return readLen;
}

static const struct proc_ops fops = {
    .proc_open = my_open,
    .proc_read = my_read,
    .proc_write = my_write,
    .proc_release = my_release
};

static int __init md_init(void)
{
    printk(KERN_INFO "+init\n");

    if ((buf = vmalloc(PAGE_SIZE)) == NULL) {
        printk(KERN_INFO "+ error vmalloc\n");
        return -ENOMEM;
    }

    memset(buf, 0, PAGE_SIZE);

    if ((pdir = proc_mkdir(mydir, NULL)) == NULL) {
        printk(KERN_INFO "+ error proc_mkdir\n");
        freeMemory();
        return -ENOMEM;
    }

    if ((pfile = proc_create(myfilename, 0666, pdir, &fops)) == NULL) {
        printk(KERN_INFO "+ error proc_create\n");
        freeMemory();
        return -ENOMEM;
    }

    if ((pslink = proc_symlink(myslink, NULL, myfpath)) == NULL) {
        printk(KERN_INFO "+ error proc_symlink\n");
        freeMemory();
        return -ENOMEM;
    }

    readInd = 0;
    writeInd = 0;

    printk(KERN_INFO "+ loaded\n");

    return 0;
}

static void __exit md_exit(void)
{
    printk(KERN_INFO "+ exit\n");
    freeMemory();
}

module_init(md_init);
module_exit(md_exit);

