#include "ThreadPool.h"

ThreadPool::ThreadPool(unsigned int numThreads) {
    threads = new thread_t[numThreads];
    assert(threads != 0);
    nThreads = numThreads;
    stop = false;
    for (int i = 0; i < nThreads; i ++) {
        thread_create_p(&threads[i], &executor, (void *)this);
    }
}

int ThreadPool::submit(void (*task)(void *), void *p) {
    // PUT YOUR CODE HERE
    //Declare new task and acquire the lock
   Task newTask = {task, p};
   thread_lock.acquire();
   //add newTask to the end of the queue
   queue.push_back(newTask);
   //call signal on the condition_variable and then release lock
   condition_variable.signal();
   thread_lock.release();



     return 0;
}

void ThreadPool::shutdown(void) {
    // PUT YOUR CODE HERE
    //signal to all the thread and flip stop from true to false
    stop = true;
    condition_variable.broadcast();

    //Join together all the threads with parent process
    for (int i = 0; i < nThreads; i ++) {
        thread_join(threads[i]);
    }
}
ThreadPool::~ThreadPool() {
    delete[] threads;
}

void ThreadPool::thread_work() {
    // PUT YOUR CODE HERE
    //intialiize new Task and run while stop is false
    Task taskWork = {};
    while(!stop)
      {
        //acquire the lock and wait while the queue is empty
        thread_lock.acquire();
        while(queue.empty())
          {
            if(stop)
              break;
            condition_variable.wait(&thread_lock);
          }
        //If stop is true then release lock and break from loop
        if(stop)
          {
            thread_lock.release();
            break;
          }
        //Assign front of queue to new Task
        taskWork = queue.front();
        //remove front of queue
        queue.pop_front();
        thread_lock.release();
        taskWork.func(taskWork.param);
      }
    thread_exit(0);
}

void * executor(void *p) {
    ThreadPool *tp = (ThreadPool *)p;
    tp->thread_work();

    thread_exit(0);
    return 0;
}
