CC = gcc
CFLAGS = -Wall

all: sched_OTHER sched_FIFO sched_RR Process_Scheduling

sched_OTHER: sched_OTHER.c
	$(CC) $(CFLAGS) -o $@ $< -lrt

sched_FIFO: sched_FIFO.c
	$(CC) $(CFLAGS) -o $@ $< -lrt

sched_RR: sched_RR.c
	$(CC) $(CFLAGS) -o $@ $< -lrt

process_scheduling: process_scheduling.c
	$(CC) $(CFLAGS) -o $@ $< -lrt

run: Process_Scheduling
	./Process_Scheduling
	python3 histogram.py


.PHONY: clean

clean:
	rm -f sched_OTHER sched_FIFO sched_RR Process_Scheduling
