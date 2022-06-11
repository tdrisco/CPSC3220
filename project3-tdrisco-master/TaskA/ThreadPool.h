#ifndef PROJECT3_THREADPOOL_H
#define PROJECT3_THREADPOOL_H

#include "thread.h"
#include "Lock.h"
#include "CV.h"
#include <deque>
#include <cassert>

struct Task {
    void (*func)(void *);
    void *param;
};

void * executor(void *p);

class ThreadPool {
public:
    ThreadPool(unsigned int numThreads);
    int submit(void (*task)(void *), void *p);
    void shutdown(void);
    ~ThreadPool();
    void thread_work();

private:
    // You can add your synchronization variables here
    //
    //Add a lock and condition variable
    Lock thread_lock;
    CV condition_variable;
    thread_t *threads;
    int nThreads;
    bool stop;
    std::deque <Task> queue;
};
#endif //PROJECT3_THREADPOOL_H
