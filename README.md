# CPSC/ECE 3200: Introduction to Operating System - Project #3

In this project, you will work on two programs that help you (1) to be familiar with the concepts 
of threads, synchronization, and multi-threaded programming design patterns; 
(2) to understand how user level thread scheduling works; and 
(3) to gain hand-on experience on writing multi-threading programs.

# Task A: Thread Pool (60 Points)

<code>Implement a Thread Pool library that can be used by a multi-threaded server program. The library shall work on a Linux machine in the School of Computing computer labs. </code>

## Overview

Today, most server programs are written as multi-threaded programs. 
Among these programs, thread pool is a widely used design pattern that solves two issues
 existed in naive programs. that creates a separate thread for each request. First, creating a threads takes time and it is a waste when the thread will be discarded once it has completed its work. Second, because the number of requests at any time point is unpredictable, if we don't place a bound on the number of concurrent threads, a large number of threads could exhaust system resources like CPU time or memory. 

The general idea behind a thread pool is to create a number of threads at start-up and then place them into a pool, where they sit and wait for work. When a server receives a request, rather than creating a thread, it instead submits the request to the thread pool and resumes waiting for additional requests. If there is an available thread in the pool, it is awakened, and the request is serviced immediately. If the pool contains no available thread, the task is queued until one becomes free. Once a thread completes its service, it returns to the pool and awaits more work. 

In this task, you will implement a thread pool library that can be used by some server programs. A server program submits its tasks to the thread pool and the threads in the thread pool fetch tasks and do the work. We provide an example server program called `tptest` and a starter code for you to get started. The starter code uses the simple thread API developed by the authors of the OSPP textbook. However, you are free to use other thread libraries like `pthread` if you are comfortable to do so.  

## Specification

1. You provide a thread pool library that implements a ThreadPool class which has at least three APIs:

   a. `ThreadPool(unsigned int numThreads)` creates a pool of numThreads.
   
   b.  `int submit(void (*somFunction)(void *), void *p)` submits a task to the pool, where `somFunction` is 
   a pointer to the function that a thread from he pool will execute and ps is parameter passed to the function.
   
   c.  `shutdown()` that shutdown the pools. 
1. Invoking the command `make` shall create a executable binary `tpooltest`, which can be used to test your thread pool implementation. 

### Example Output
Your program shall generate output like the following example except that the order of the output may be different. 
```
$ make
g++ -c  -g -Wall -Werror -D_POSIX_THREAD_SEMANTICS tpooltest.cc -o tpooltest.o
g++ -c  -g -Wall -Werror -D_POSIX_THREAD_SEMANTICS ThreadPool.cc -o ThreadPool.o
g++ -c  -g -Wall -Werror -D_POSIX_THREAD_SEMANTICS Lock.cc -o Lock.o
g++ -c  -g -Wall -Werror -D_POSIX_THREAD_SEMANTICS CV.cc -o CV.o
gcc -c  -g -Wall -Werror -D_POSIX_THREAD_SEMANTICS thread.c -o thread.o
g++  -g -Wall -Werror -D_POSIX_THREAD_SEMANTICS tpooltest.o ThreadPool.o Lock.o CV.o thread.o -lpthread -o tpooltest
$ ./tpooltest 3 0 5
sum of 1 ... 1 = 1
sum of 1 ... 4 = 10
sum of 1 ... 5 = 15
sum of 1 ... 2 = 3
sum of 1 ... 3 = 6
sum of 3 ... 9 = 42
sum of 6 ... 8 = 21
sum of 9 ... 10 = 19
sum of 7 ... 12 = 57
sum of 3 ... 8 = 33
$
```

## Hints

+ If you use the simple thread API, then you should apply the best practice of designing shared objects described in the OSPP textbook. 
The textbook covers all the APIs for the thread, lock, and condition variable that you will need for this task.   

+ We have provide a starter code in `ThreadPool.cc` and `ThreadPool.h`. We also provided a `tpooltest.cc` for 
an example server program.

+ In `ThreadPool.h`, you will see some unusual treatment: the `ThreadPool` constructor (ThreadPool.cc:9) 
passes an external function `executor()` and a pointer to itself (i.e., `this`) 
to `thread_create_p()` and then `executor` calls the `thread_work()` method 
of the ThreadPool object pointed by the pointer parameter passed into the `executor()` 
in `thread_create_p`. The only reason for this treatment is to bypass 
the constraint that ISO C++ forbids taking the address of an unqualified or 
parenthesized non-static member function to form a pointer to member function. We provide the code 
so that you can avoid this error and know how to fix it when you see similar errors in your code.   

+ For the task queue, you can use the `deque` template library, although you have 
to add proper synchronizations to make your queue thread-safe. For this task, all you need 
for deque is as follows:  

    + `#include <deque>` include library for deque implementation
    + `std::deque <Task> queue;`  define a queue of Tasks
    + `queue.push_back(task);`    add a task to the queue
    +  `task = queue.front();`    fetch a task in the front      
       `queue.pop_front();`       remove a task from the front
    +  `queue.size()`             get number of tasks in the queue  

# Task B: User-level threads on xv6 (40 Points)
<code>Implement thread switching in user-level thread package. The program shall run on the xv6-riscv OS in the QEMU environment    
in the School of Computing computer labs. </code>

## Overview

This task will familiarize you with how state is saved and restored in context switches. You will implement switching between 
threads in a user-level threads package. This task involves more code reading and concept understanding. 
Once you truly understand the concepts of how user-level threads work, the code will be trivial. 

### Warmup: RISC-V assembly (0 points)

For this task it will be important to understand a bit of RISC-V assembly. There is a file `user/call.c` in
 your xv6 repo. `make fs.img` builds a user program `call`
  and a readable assembly version of the program in `user/call.asm`.

Read the code in `call.asm` for the functions `g`, `f`, and `main`. 
The instruction manual for RISC-V is available at https://content.riscv.org/wp-content/uploads/2017/05/riscv-spec-v2.2.pdf. 
Here are some questions that you should answer (store the answers in a file `answers-syscall.txt` in the TaskB folder):

1. Which registers contain arguments to functions? For 
example, which register holds 13 in `main`'s call to `printf`?
1.  Where is the function call to `f` from `main`? 
Where is the call to `g`? (Hint: the compiler may inline functions.)
1. At what address is the function `printf` located?
1. What value is in the register `ra` just after the `jalr` to `printf` in main?

### Uthread: switching between threads (40 points)
In this task you will design the context switch mechanism for 
a user-level threading system, and then implement it. 
To get you started, your xv6 has two files user/uthread.c and user/uthread_switch.S, and 
a rule in the Makefile to build a uthread program. uthread.c contains most of a user-level threading package, and code for three simple test threads. The threading package is missing some of the code to create a thread and to switch between threads.

Your job is to come up with a plan to create threads and save/restore registers to 
switch between threads, and implement that plan.

Once you've finished, you should see the following output when you 
run uthread on xv6 (the three threads might start in a different order):
$ make qemu
...
$ uthread
thread_a started
thread_b started
thread_c started
thread_c 0
thread_a 0
thread_b 0
thread_c 1
thread_a 1
thread_b 1
...
thread_c 99
thread_a 99
thread_b 99
thread_c: exit after 100
thread_a: exit after 100
thread_b: exit after 100
thread_schedule: no runnable threads
$
This output comes from the three test threads, each of 
which has a loop that prints a line and then yields the CPU to the other threads.

At this point, however, with no context switch code, you'll see no output.

1. You should complete `thread_create` to create a 
properly initialized thread so that when the scheduler switches to 
that thread for the first time, `thread_switch` returns to the function 
passed as argument `func`, running on the thread's stack. 
You will have to decide where to save/restore registers. 
Several solutions are possible. 
You are allowed to modify struct thread. 
1. You'll need to add a call to `thread_switch` in `thread_schedule`; 
you can pass whatever arguments you need to `thread_switch`, 
but the intent is to switch from thread `t` to the `next_thread`.
1. You will need to complete `thread_switch` function in the `uthread_switch.S` file.

Some hints:

+ `thread_switch` needs to save/restore only the callee-save registers. Why?
+ You can add fields to `struct thread` into which to save registers.
+ You can see the assembly code for `uthread` in `user/uthread.asm`, which may be handy for debugging.
+ To test your code it might be helpful to single step through your `thread_switch` using 
`riscv64-linux-gnu-gdb`. 

### `gdb` debug example
You can get started in this way:

```
(gdb) file user/_uthread
Reading symbols from user/_uthread...
(gdb) b thread.c:60
```
This sets a breakpoint at a specified line in `thread.c`. 
The breakpoint may (or may not) be triggered before you even run `uthread`. 
How could that happen?

Once your xv6 shell runs, type "`uthread`", and gdb will break 
at line `thread_switch`. Now you can type commands like the following 
to inspect the state of `uthread`:

```
  (gdb) p/x *next_thread
```
With "x", you can examine the content of a memory location:
```
  (gdb) x/x next_thread->stack
```
You can single step assembly instructions using:
```
   (gdb) si
```  
On-line documentation for gdb is [here](https://sourceware.org/gdb/current/onlinedocs/gdb/).

## Submission
Please following the procedure below in your submission.

1. Switch to your project folder: 
```cd ~/path/to/your-projects-folder```
2. Clean up the folders
    ```
    cd TaskA
    make clean
    cd ../TaskB
    make clean
    ```
3. List files that you have added or modified: ```git status```
4. Stage the new and modified  files in your local repo: ```git add files-you-created-or-modified```
5.  Commit the changes to your local repo: ```git commit -m "Finished Project #2"```
6. Push the changes to the remote repo (i.e. github): ```git push``` Or ```git push origin master```. 
7. Verify all changes have been push into the repo on github.
    + You can run "git status", "git log", and "git show" to the see the changes and commits you have made
    + You can also log into github to see if your changes have been actually pushed into your project repo on github.
8. (Optional but recommended)  For a further validation, you can check out the repo in a separate folder 
    on your computer and then verify that it has all the files for the programs to work correctly.
