include <linux/sched/signal.h>
#include <linux/init.h>
#include <linux/pid_namespace.h>
#include <linux/cdeu.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Param Kumar Raman");
MODULE_DESCRIPTION("Counting No. of running processes");

void count_running_processes(void) {
  int count = 0;
  struct task_struct *task;
  for_each_process(task) {
    count++;
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

module_init(proc_init);_
module_exit(proc_cleanup);
