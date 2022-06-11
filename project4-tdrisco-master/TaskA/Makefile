BINARIES := jssim loadgen
all: $(BINARIES)

CFLAGS := $(CFLAGS) -std=c++11 -g -Wall -Werror

clean:
	rm -f *.o $(BINARIES)

tags:
	etags *.h *.c *.cc

%.o: %.cc
	g++ -c $(CFLAGS) $< -o $@

%.o: %.c
	gcc -c $(CFLAGS) $< -o $@

jssim.o: jssim.cc task.h

task.o: task.cc task.h

fifo.o: fifo.cc task.h sched.h

rr.o: rr.cc task.h sched.h

srtf.o: srtf.cc task.h sched.h

mlfq.o: mlfq.cc task.h sched.h

jssim: jssim.o task.o events.o fifo.o rr.o mlfq.o srtf.o

loadgen: loadgen.o
	g++ $(CFLAGS) $^ -o $@

jssim: jssim.o task.o events.o fifo.o rr.o srtf.o mlfq.o
	g++ $(CFLAGS) $^ -o $@

