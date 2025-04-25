#include <linux/init_task.h>
#include <linux/module.h>
#include <linux/sched.h>
// #include <linux/fs_struct.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kathrine");

static void print_task_flags(unsigned int flags)
{
    printk(KERN_CONT " flags: [");

    // Проверка каждого флага по порядку
    if (flags & PF_VCPU)             printk(KERN_CONT "VCPU ");
    if (flags & PF_IDLE)             printk(KERN_CONT "IDLE ");
    if (flags & PF_EXITING)          printk(KERN_CONT "EXITING ");
    if (flags & PF_IO_WORKER)        printk(KERN_CONT "IO_WORKER ");
    if (flags & PF_WQ_WORKER)        printk(KERN_CONT "WQ_WORKER ");
    if (flags & PF_FORKNOEXEC)       printk(KERN_CONT "FORKNOEXEC ");
    if (flags & PF_MCE_PROCESS)      printk(KERN_CONT "MCE_PROCESS ");
    if (flags & PF_SUPERPRIV)        printk(KERN_CONT "SUPERPRIV ");
    if (flags & PF_DUMPCORE)         printk(KERN_CONT "DUMPCORE ");
    if (flags & PF_SIGNALED)         printk(KERN_CONT "SIGNALED ");
    if (flags & PF_MEMALLOC)         printk(KERN_CONT "MEMALLOC ");
    if (flags & PF_NPROC_EXCEEDED)   printk(KERN_CONT "NPROC_EXCEEDED ");
    if (flags & PF_USED_MATH)        printk(KERN_CONT "USED_MATH ");
    if (flags & PF_NOFREEZE)         printk(KERN_CONT "NOFREEZE ");
    if (flags & PF_KSWAPD)           printk(KERN_CONT "KSWAPD ");
    if (flags & PF_MEMALLOC_NOFS)    printk(KERN_CONT "MEMALLOC_NOFS ");
    if (flags & PF_MEMALLOC_NOIO)    printk(KERN_CONT "MEMALLOC_NOIO ");
    if (flags & PF_LOCAL_THROTTLE)   printk(KERN_CONT "LOCAL_THROTTLE ");
    if (flags & PF_KTHREAD)          printk(KERN_CONT "KTHREAD ");
    if (flags & PF_RANDOMIZE)        printk(KERN_CONT "RANDOMIZE ");
    if (flags & PF_NO_SETAFFINITY)   printk(KERN_CONT "NO_SETAFFINITY ");
    if (flags & PF_MCE_EARLY)        printk(KERN_CONT "MCE_EARLY ");
    if (flags & PF_MEMALLOC_PIN)     printk(KERN_CONT "MEMALLOC_PIN ");
    if (flags & PF_SUSPEND_TASK)     printk(KERN_CONT "SUSPEND_TASK ");

    printk(KERN_CONT "]\n");
}

static int __init mod_init(void)
{
    printk(KERN_INFO " + module is loaded.\n");
    struct task_struct *task = &init_task;
    do
    {
        printk(KERN_INFO "+pid=%5d, comm=%15s, ppid=%5d, parentcomm=%15s, on_cpu=%5d, policy=%d, prio=%5d, state=%5d, exit_state=%d, exit_code=%d, exit_signal=%5d, utime=%10llu, stime=%10llu", 
            task->pid, 
            task->comm, 
            task->parent->pid, 
            task->parent->comm, 
            task->on_cpu, 
            task->policy,
            task->prio, 
            // task->static_prio,
            // task->normal_prio,
            // task->rt_priority,
            task->__state, 
            // task_state_to_char(task), 
            task->exit_state, 
            task->exit_code, 
            task->exit_signal,
            task->utime, 
            task->stime);
            print_task_flags(task->flags);             
    } while ((task = next_task(task)) != &init_task);

    printk(KERN_INFO "+pid=%5d, comm=%15s, ppid=%5d, parentcomm=%15s, on_cpu=%5d, policy=%d, prio=%5d, state=%5d, exit_state=%d, exit_code=%d, exit_signal=%5d, utime=%10llu, stime=%10llu", 
            current->pid, 
            current->comm, 
            current->parent->pid, 
            current->parent->comm, 
            current->on_cpu, 
            current->policy,
            current->prio, 
            // current->static_prio,
            // current->normal_prio,
            // current->rt_priority, // static_prio=%5d, normal_prio=%5d, rt_priority=%5d, 
            current->__state, 
            // task_state_to_char(current), 
            current->exit_state, 
            current->exit_code, 
            current->exit_signal,
            current->utime, 
            current->stime);
            print_task_flags(current->flags);    
    return 0;
}

static void __exit mod_exit(void)
{
    printk(KERN_INFO " + module is unloaded.\n");
}

module_init(mod_init);
module_exit(mod_exit);