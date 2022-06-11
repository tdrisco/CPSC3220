# CPSC/ECE 3200: Introduction to Operating System - Project #4

This project consists of two task A. Task A helps you to understand the typical scheduling algorithms and their implementation. Task B helps you to be familiar with page allocation and memory management. 

# Task A: Job Scheduling Simulator (50 Points)

<code>Implement a scheduler simulator for Round Robin and MLFQ schedulers.</code>

## Overview
In this task, you implement a scheduler simulator `jssim` to support two schedulers: Round Robin and MLFQ. The usage of jssim is as follows:

```
	./jssim -p [policy] -q [list-of-quantums] -w [workload]
```
Here, 

+ policy is `fifo`, `rr`, `srtf`, or `mlfq`.
+ workload is the filename for a workload formatted as follows:
	```
	#ID,Arrival,Service
	1   0   2
	2   5   1
	3   8   7
	```
+ list-of-quantums is one or more integers for the time quantum(s) for Round Robin or MLFQ scheduler.

Example usages are like follows:

```
	make
	./jssim -p fifo -w data/w1
	./jssim -p rr -q 2 -w data/w1
	./jssim -p mlfq -q 2 4 -w data/w1
	./jssim -p srtf -w data/w1	
```

For you to focus on the scheduler implementation, we have provided a simulator framework as well as an implementation of FIFO and SRTF (Shortest Remaining Time First) scheduler. 
You can either use the provided framework in your implementation or write your own implementation from scratch. 

The point distribution is as follows:

+ Round Robin: 20 points
+ MLFQ: 30 points

## Specification

1. At minimum, your simulator should take a workload file <input> and write the scheduling results into a text file named <input>-<policy>.result. The scheduling results is formatted as follows:
	```
	#ID,Arrival,Service,Departure,Response,Wait
	1 0 13 33 33 20
	2 2 8 22 20 12
	3 2 12 32 30 18
	```
1. For the MLFQ scheduler, use two queues: one high-priority queue for interactive tasks and one low-priority queue for CPU-bound tasks, each serviced in a round-robin fashion.
   a. Each task begins in a high-priority queue; if its quantum expires before its CPU burst ends, it is demoted to the low-priority queue.
   b. A new arriving task preempts a running low-priority task because it has a higher priority. It does not preempt a running high-priority task. 

## Hints

+ Implement Round Robin scheduler first and then work on the MLFQ scheduler.
+ Keep track of the current simulation time and all scheduling events. Implement appropriate scheduling action for every scheduling event.
+ If your program is based on the starter code we have provided, study the FIFO and SRTFScheduler implementation and then use it as a reference when you implement other schedulers. 
+ Design some simple test cases to test if your schedulers behave correctly.
+ Use the provided sample output as a base to validate your implementation. 

# Task B: Copy-on-Write Fork for xv6 (50 Points)
<code> Your task is to implement copy-on-write fork in the xv6 kernel. You are done if your modified kernel executes both the cowtest and usertests programs successfully. </code>

## The problem

The `fork()` system call in xv6 copies all of the parent process's user-space memory into the child. If the parent is large, copying can take a long time. In addition, the copies often waste memory; in many cases neither the parent nor the child modifies a page, so that in principle they could share the same physical memory. The inefficiency is particularly clear if the child calls `exec()`, since `exec()` will throw away the copied pages, probably without using most of them. On the other hand, if both parent and child use a page, and one or both writes it, a copy is truly needed. 

## The solution

The goal of copy-on-write (COW) fork() is to defer allocating and copying physical memory pages for the child until the copies are actually needed, if ever.

`COW fork()` creates just a pagetable for the child, with PTEs (page table entries) for user memory pointing to the parent's physical pages. COW fork() marks all the user PTEs in both parent and child as not writable. When either process tries to write one of these COW pages, the CPU will force a page fault. The kernel page-fault handler detects this case, allocates a page of physical memory for the faulting process, copies the original page into the new page, and modifies the relevant PTE in the faulting process to refer to the new page, this time with the PTE marked writeable. When the page fault handler returns, the user process will be able to write its copy of the page.

`COW fork()` makes freeing of the physical pages that implement user memory a little trickier. A given physical page may be referred to by multiple processes' page tables, and should be freed only when the last reference disappears. 

## The cowtest program

To help you test your implementation, we've provided an xv6 program called cowtest (source in user/cowtest.c). cowtest runs various tests, but even the first will fail on unmodified xv6. Thus, initially, you will see: 

```
$ cowtest
simple: fork() failed
$ 
```

The "simple" test allocates more than half of available physical memory, and then fork()s. The fork fails because there is not enough free physical memory to give the child a complete copy of the parent.

When you are done, your kernel should be able to run both cowtest and usertests. That is:

```
$ cowtest
simple: ok
simple: ok
three: zombie!
ok
three: zombie!
ok
three: zombie!
ok
file: ok
ALL COW TESTS PASSED
$ usertests
...
ALL TESTS PASSED
$
```

## Hints
See the Chapter 3 of the xv6 book, the homework and to leture as well to get familiar with xv6 kernel code that's relevent for copy-on-write.

Here's one reasonable plan of attack. 

+ Modify uvmcopy() in `kernel/vm.c` to map the parent's physical pages into the child, instead of allocating new pages, and clear PTE_W in the PTEs of both child and parent. 

+ Modify usertrap() in `kernel/trap.c` to recognize page faults. When a page-fault occurs on a COW page, allocate a new page with kalloc(), copy the old page to the new page, and install the new page in the PTE with PTE_W set. 

+ Next, ensure that each physical page is freed when the last PTE reference to it goes away (but not before!), perhaps by implementing reference counts in `kalloc.c`. 

+ Finally, modify copyout() `kernel/vm.c` to use the same scheme as page faults when it encounters a COW page.

+ It may be useful to have a way to record, for each PTE, whether it is a COW mapping. You can use the RSW (reserved for software) bits in the RISC-V PTE for this.

+ `usertests` explores more situations than cowtest, so don't forget to check that all tests pass for both.

+ Some helpful macros and definitions for page table flags are at the end of the file kernel/riscv.h. 

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
