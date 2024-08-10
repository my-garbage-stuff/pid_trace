#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/sched.h>

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    struct task_struct *task = current;
    printk(KERN_INFO "New PID created: %d, Command: %s\n", task->pid, task->comm);
    return 0; // Return 0 to continue execution
}

static void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
    // You can add post-processing code here if needed
}

static struct kprobe kp;

static int __init trace_pid_init(void)
{
    
    kp.symbol_name = "__set_task_comm";
    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;

    if (register_kprobe(&kp) < 0) {
        printk(KERN_ERR "Failed to register kprobe\n");
        return -1;
    }

    printk(KERN_INFO "Kprobe registered for __set_task_comm\n");
    return 0;
}

static void __exit trace_pid_exit(void)
{
    unregister_kprobe(&kp);
    printk(KERN_INFO "Kprobe unregistered\n");
}

module_init(trace_pid_init);
module_exit(trace_pid_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple kernel module to trace new PID creation");

