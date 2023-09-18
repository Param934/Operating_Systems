#include <stdio.h>
#include <stdint.h>
#include <sched.h>

int main() {
    struct sched_param param;
    param.sched_priority = 0; // Default Priority for SCHED_OTHER
     if (sched_setscheduler(0, SCHED_OTHER, &param) == -1) {
            perror("sched_setscheduler");
            exit("EXIT_FAILURE");
        }
    uint64_t count = 0;
    for (uint64_t i = 1; i <= (1ULL << 32); i++) {
        count++;
    }

    // printf("Count: %llu\n", count);
    return 0;
}
