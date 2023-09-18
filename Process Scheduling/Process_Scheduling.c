#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <time.h>

int main() {
    pid_t pid;
    int status;
    struct timespec dummy,start, end;
    double elapsed;
    FILE *ftr;
    ftr=fopen("policy_time.txt","w");

    // Create a child process with SCHED_OTHER policy (standard priority)
    clock_gettime(CLOCK_MONOTONIC, &dummy);
    clock_gettime(CLOCK_MONOTONIC, &start);
    pid = fork();

    if (pid == 0) {
        // Child Process 1 - SCHED_OTHER
        execl("./sched_OTHER", "sched_OTHER", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent Process
        waitpid(pid, &status, 0);
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed = (10e9 *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec)/(double)10e9;
        fprintf(ftr,"SCHED_OTHER %.6lf\n",(double)elapsed );
        printf("SCHED_OTHER completed\n");
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // Create a child process with SCHED_RR policy (default priority)
    clock_gettime(CLOCK_MONOTONIC, &start);
    pid = fork();

    if (pid == 0) {
        // Child Process 2 - SCHED_RR
        execl("./sched_RR", "sched_RR", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent Process
        waitpid(pid, &status, 0);
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed = (10e9 *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec)/(double)10e9;
        fprintf(ftr,"SCHED_RR %.6lf\n",(double)elapsed );
        printf("SCHED_RR completed\n");
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // Create a child process with SCHED_FIFO policy (default priority)
    clock_gettime(CLOCK_MONOTONIC, &start);
    pid = fork();

    if (pid == 0) {
        // Child Process 3 - SCHED_FIFO
        execl("./sched_FIFO", "sched_FIFO", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent Process
        waitpid(pid, &status, 0);
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed = (10e9 *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec)/(double)10e9;
        fprintf(ftr,"SCHED_FIFO %.6lf\n",(double)elapsed );
        printf("SCHED_FIFO completed\n");
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    fclose(ftr);
    return 0;
}
