#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/slab.h>

static char *get_process_cmdline(struct task_struct *task) {
    struct mm_struct *mm;
    char *cmdline;
    unsigned long len;

    mm = get_task_mm(task);
    if (!mm) {
        printk(KERN_ERR "Failed to get memory descriptor for task PID: %d\n", task->pid);
        return NULL;
    }

    len = mm->arg_end - mm->arg_start;
    if (len == 0) { // Check if the command line length is zero
        printk(KERN_INFO "Command line is empty for PID: %d\n", task->pid);
        mmput(mm);
        return NULL;
    }

    cmdline = kmalloc(len + 1, GFP_KERNEL);
    if (!cmdline) {
        printk(KERN_ERR "Failed to allocate memory for command line of PID: %d\n", task->pid);
        mmput(mm);
        return NULL;
    }

    if (copy_from_user(cmdline, (char __user *)mm->arg_start, len)) {
        printk(KERN_ERR "Failed to copy command line from user space for PID: %d\n", task->pid);
        kfree(cmdline);
        mmput(mm);
        return NULL;
    }

    cmdline[len] = '\0'; // Null-terminate the string
    mmput(mm);
    return cmdline;
}

static char* blocklist[] = {
    "amagas",
};

static int handler_pre(struct kprobe *p, struct pt_regs *regs) {
    struct task_struct *task = current;
    char *cmdline = get_process_cmdline(task);

    if (cmdline) {
        printk(KERN_INFO "New PID created: %d, Command: %s\n", task->pid, cmdline);
    } else {
        printk(KERN_INFO "New PID created: %d, Command: (empty)\n", task->pid);
        return 0;
    }

    int cmp;
    for (size_t i=0;i<sizeof(blocklist)/ sizeof(char*);i++){
        cmp = strncmp(blocklist[i], cmdline, strlen(cmdline));
        printk(KERN_INFO "%s %s %d\n",
            blocklist[i],
            cmdline,
            cmp
        );
        // TODO: block if unwanted
    }
    if (cmdline) {
        kfree(cmdline); // Free the allocated memory after use
    }
    return 0; // Return 0 to continue execution
}

static void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {
    // Post-handler can be used for additional logging if needed
}

static struct kprobe kp;

static int __init trace_pid_init(void) {
    kp.symbol_name = "finalize_exec";
    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;

    if (register_kprobe(&kp) < 0) {
        printk(KERN_ERR "Failed to register kprobe for finalize_exec\n");
        return -1;
    }

    printk(KERN_INFO "Kprobe registered for finalize_exec\n");
    return 0;
}

static void __exit trace_pid_exit(void) {
    unregister_kprobe(&kp);
    printk(KERN_INFO "Kprobe unregistered\n");
}

module_init(trace_pid_init);
module_exit(trace_pid_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple kernel module to trace new PID creation");
