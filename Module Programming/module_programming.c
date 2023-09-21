#include <linux/sched/signal.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Param Kumar Raman");
MODULE_DESCRIPTION("Counting No. of running processes");

void count_running_processes(void) {
    int count = 0;
    struct task_struct *task;
    for_each_process(task) {
        if (task -> __state == TASK_RUNNING) count++;
    }
    printk(KERN_INFO "Number of Running Processes = %d\n", count);
}

int proc_init(void) {
    printk(KERN_INFO "Kernel Module Loaded\n");
    count_running_processes();
    return 0;
}

void proc_cleanup(void) {
  printk(KERN_INFO "Kernel Module Removed\n");
}

module_init(proc_init);
module_exit(proc_cleanup);
