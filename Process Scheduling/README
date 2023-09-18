# Operating_Systems
**Process Scheduling**

This exercise is to show you how to use Linux’s scheduling policies for different processes. You will be creating three processes, where each of the three processes will count from 1 to 2^32. The three processes should be created with fork() and thereafter the child processes should use execl() family system calls to run another C file which will do the counting mentioned above.


Reiterating you need to launch three process, each of which calls another process(the counting process)

The following should be the detailed specification of each of the process, to being with:

1. Process 1 : Uses SCHED OTHER scheduling discipline with standard priority (nice:0).

2. Process 2 : Uses SCHED RR scheduling discipline with default priority.

3. Process 3 : Uses SCHED FIFO scheduling discipline with default priority.


Each of these processes must time the process of counting from 1 to 2^32. To time the execution, the parent process could get the clock timestamp (using clock_gettime()) before the fork and after each process terminates (the event of which could be identified when the blocking system call waitpid() returns).


We require you to benchmark these three schedulers by changing the scheduling classes of the three processes. You would require to generate histograms showing which scheduler completes the task when, depending upon the scheduling policy. You may choose different colours for the histogram bars, with one axis showing the time to complete, and the other showing the scheduling priorities. (You may use python to plot the histogram.)
