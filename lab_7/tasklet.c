#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <asm/io.h>


#define IRQ_NUM 1
#define SCANCODE_PORT 0x60
#define STATUS_PORT 0x64
#define FILENAME "my_tasklet"

MODULE_LICENSE("GPL");


static struct tasklet_struct *my_tasklet;
static struct proc_dir_entry *file;
static int counter = 0;
static char *last_symbol = "No symbol";
static ktime_t start_tasklet_time = 0;
static ktime_t last_tasklet_time = 0;

#define len_symbs 58
static char * symbs[] =  {
    " ", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
    "-", "+", "Backspace", 
    "Tab", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]",
    "Enter", "Ctrl",
    "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "\"", "'",
    "Left Shift", "|", 
    "z", "x", "c", "v", "b", "n", "m", "<", ">", "?", "Right Shift", 
    "*", "Alt", "Space", 
};

static void my_tasklet_func(unsigned long data) {
    char code = ((char) data) & 0x7F;
    last_symbol = symbs[code];
    printk(KERN_INFO "+ tasklet: code=%d, symbol=%s\n", code, last_symbol);
    counter++;
    my_tasklet->state = TASKLET_STATE_SCHED;
    ktime_t now = ktime_get();
    last_tasklet_time = now - start_tasklet_time;
}

static irqreturn_t my_handler(int irq, void *dev_id) {
    if (IRQ_NUM == irq) {
        char scancode = inb(SCANCODE_PORT);
        if (scancode >= 0 && scancode < len_symbs) {
            start_tasklet_time = ktime_get();
            my_tasklet->data = (unsigned long) scancode;
            tasklet_schedule(my_tasklet);
            return IRQ_HANDLED;
        }
    }
    return IRQ_NONE;
}

static int my_single_show(struct seq_file *s, void *v) {
    seq_printf(s, "Counter: %d\n", counter);
    seq_printf(s, "Symbol: %s\n", last_symbol);
    seq_printf(s, "Tasklet time: %llu ns\n", last_tasklet_time);
    return 0;
}

static int single_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, my_single_show, NULL);
}

static const struct proc_ops single_proc_fops = {
    .proc_open = single_proc_open,
    .proc_release = single_release,
    .proc_read = seq_read,
};


static int __init my_init(void) {
    printk(KERN_INFO "+ tasklet: init\n");
    
    file = proc_create(FILENAME, 0666, NULL, &single_proc_fops);
    if (file == NULL) {
        printk(KERN_ERR "singlefile: proc_create failed\n");
        return -ENOMEM;
    }

    int err = request_irq(IRQ_NUM, my_handler, IRQF_SHARED, "my_handler", &my_handler);
    if (err) {
        printk(KERN_ERR "+ tasklet: request_irq failed\n");
        return -ENOMEM;
    }

    my_tasklet = kmalloc(sizeof(struct tasklet_struct), GFP_KERNEL);
    if (!my_tasklet)
    {
        printk(">> my_tasklet: ERROR kmalloc!\n");
        return -1;
    }
    tasklet_init(my_tasklet, my_tasklet_func, 0);
    if (!my_tasklet) {
        printk(KERN_ERR "+ tasklet: tasklet_init failed\n");
        return -ENOMEM;
    }
    return 0;
}

static void __exit my_exit(void) {
    printk(KERN_INFO "+ tasklet: exit\n");
    free_irq(IRQ_NUM, &my_handler);
    tasklet_kill(my_tasklet);
    proc_remove(file); 
}

module_init(my_init);
module_exit(my_exit);