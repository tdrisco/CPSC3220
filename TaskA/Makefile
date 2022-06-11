BINARIES := tpooltest
all: $(BINARIES)

CFLAGS := $(CFLAGS) -g -Wall -Werror -D_POSIX_THREAD_SEMANTICS

THREADLIBS := Lock.o CV.o thread.o
CTHREADLIBS := thread.o

clean:
	rm -f *.o $(BINARIES)

tags:
	etags *.h *.c *.cc

%.o: %.cc
	g++ -c $(CFLAGS) $< -o $@

%.o: %.c
	gcc -c $(CFLAGS) $< -o $@

tpooltest: tpooltest.o ThreadPool.o $(THREADLIBS) $(CTHREADLIBS)
	g++ $(CFLAGS) $^ -lpthread -o $@

