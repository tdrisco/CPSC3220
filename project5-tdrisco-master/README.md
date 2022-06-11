# CPSC/ECE 3200: Introduction to Operating System - Project #5

This project consists of two tasks that you will work on XV6. 
Task A aims to help you understand how to increase parallelism on a multicore architecture. 
Task B aims to help you understand the index structure and the support of large files on XV6.

# Task A: Per-Process Memory Allocator (50 Points)

<code>Re-designing memory allocator code in XV6 to increase parallelism.</code>

## Overview -- Locks and Memory Allocator
A common symptom of poor parallelism on multi-core machines is high lock contention. 
Improving parallelism often involves changing both data structures and locking strategies 
in order to reduce contention. You'll do this for the xv6 memory allocator.

Before writing code, you should make sure you have read 
"Chapter 4: Locking" from the xv6 book and studied the corresponding code.

The program user/kalloctest stresses xv6's memory allocator: 
three processes grow and shrink their address spaces, resulting in many calls to kalloc and kfree. 
kalloc and kfree obtain kmem.lock. 
kalloctest prints the number of test-and-sets that did not succeed in 
acquiring the kmem lock (and some other locks), which is a rough measure of contention:

```
    $ kalloctest
    start test0
    test0 results:
    === lock kmem/bcache stats
    lock: kmem: #test-and-set 161724 #acquire() 433008
    lock: bcache: #test-and-set 0 #acquire() 812
    === top 5 contended locks:
    lock: kmem: #test-and-set 161724 #acquire() 433008
    lock: proc: #test-and-set 290 #acquire() 961
    lock: proc: #test-and-set 240 #acquire() 962
    lock: proc: #test-and-set 72 #acquire() 907
    lock: proc: #test-and-set 68 #acquire() 907
    test0 FAIL
    start test1
    total allocated number of pages: 200000 (out of 32768)
    test1 OK
```

`acquire` maintains, for each lock, the count of calls to `acquire` for that lock, 
and the count of test-and-sets that didn't manage to acquire the lock. 
kalloctest calls a system call that causes the kernel to print those counts 
for the kmem and bcache locks and for the 5 most contended locks. 

If there is lock contention the number of test-and-sets will be high 
because it takes many loop iterations before acquire obtains the lock. 
The system call returns the sum of the #test-and-sets for the kmem and bcache locks.

For this task, you must use a dedicated unloaded machine with multiple cores. All the
SoC lab machines will meet this requirement.

The root cause of lock contention in kalloctest is that kalloc() has a 
single free list, protected by a single lock. To remove lock contention, 
you will have to redesign the memory allocator to avoid a single lock and list.
The basic idea is to maintain a free list per CPU, each list with its own lock.
Allocations and frees on different CPUs can run in parallel, b
ecause each CPU will operate on a different list. 
The main challenge will be to deal with the case in which one CPU's 
free list is empty, but another CPU's list has free memory; in that case,
 the one CPU must "steal" part of the other CPU's free list. 
 Stealing may introduce lock contention, but that will hopefully be infrequent. 

## Specification

+ Your job is to implement per-CPU freelists and stealing when one CPU's list is empty. 
+ Run kalloctest to see if your implementation has reduced lock contention,
 and to check that it can still allocate all of memory. 
+ Your output will look similar to that shown below, although the specific numbers 
will differ. 
+ Make sure usertests still passes.

```
    $ kalloctest
    start test0
    test0 results:
    === lock kmem/bcache stats
    lock: kmem: #test-and-set 0 #acquire() 33167
    lock: kmem: #test-and-set 0 #acquire() 200114
    lock: kmem: #test-and-set 0 #acquire() 199752
    lock: bcache: #test-and-set 0 #acquire() 28
    === top 5 contended locks:
    lock: proc: #test-and-set 22303 #acquire() 180082
    lock: proc: #test-and-set 4162 #acquire() 180130
    lock: proc: #test-and-set 4115 #acquire() 180129
    lock: proc: #test-and-set 342 #acquire() 180070
    lock: proc: #test-and-set 39 #acquire() 180070
    test0 OK
    start test1
    total allocated number of pages: 200000 (out of 32768)
    test1 OK
    $
    $ usertests
    ...
    ALL TESTS PASSED
    $
```

## Hints
 + Please give all of your locks `initlock` names that start with "kmem".
 + You can use the constant `NCPU` from kernel/param.h
 + Let `freerange` give all free memory to the CPU running `freerange`.
 + The function `cpuid` returns the current core number, but it's only safe to 
 call it and use its result when interrupts are turned off. 
 + You should use push_off() and pop_off() to turn interrupts off and on.
 
 You can run `./grade-kalloc` to see your potential grade for this task.

# Task B: Large Files (50 Points)
<code>Implement Large Files Support using doubly-indirect blocks on XV6</code>

## Overview - Large Files

In this task, you'll increase the maximum size of an xv6 file. 
Currently xv6 files are limited to 268 blocks, or 268*BSIZE bytes (BSIZE is 1024 in xv6). 
This limit comes from the fact that an xv6 inode contains 12 "direct" block numbers and 
one "singly-indirect" block number, which refers to a block that holds up to 256 more block numbers,
 for a total of 12+256=268 blocks.

Before writing code, you should read "Chapter 7: File system" from the xv6 book and study the corresponding code.

The `bigfile` user program creates the longest file it can, and reports that size:
```
$ bigfile
..
wrote 268 blocks
bigfile: file is too small
$
```
The test fails because the longest file is only 268 blocks.

You'll change the xv6 file system code to support a "doubly-indirect" block in each inode, 
containing 256 addresses of singly-indirect blocks, each of which can contain up to 256 addresses of data blocks. 
The result will be that a file will be able to consist of up to 256*256+256+11 blocks 
(11 instead of 12, because we will sacrifice one of the direct block numbers for the double-indirect block).


### Preliminaries

`mkfs` initializes the file system to have fewer than 2000 free data blocks, 
too few to show off the changes you'll make. Modify kernel/param.h to change FSSIZE from 2000 to 200,000:

```
    #define FSSIZE       200000  // size of file system in blocks
```

Rebuild `mkfs` so that is produces a bigger disk: `$ rm mkfs/mkfs fs.img; make mkfs/mkfs`


### What to Look At
+ The format of an on-disk inode is defined by struct dinode in fs.h. 
You're particularly interested in NDIRECT, NINDIRECT, MAXFILE, and the addrs[] element of struct dinode. 
Look at Figure 7.3 in the xv6 book for a diagram of the standard xv6 inode.

+ The code that finds a file's data on disk is in `bmap()` in fs.c. 
Have a look at it and make sure you understand what it's doing. 
`bmap()` is called both when reading and writing a file. 
When writing, `bmap()` allocates new blocks as needed to hold file content,
 as well as allocating an indirect block if needed to hold block addresses.

`bmap()` deals with two kinds of block numbers. The `bn` argument is a "logical block number" -- a block number 
within the file, relative to the start of the file. The block numbers in `ip->addrs[]`, and the argument to `bread()`,
 are disk block numbers. You can view `bmap()` as mapping a file's logical block numbers into disk block numbers.

## Specification

+ Your job is to modify bmap() so that it implements a doubly-indirect block, in addition to direct blocks and a singly-indirect block. 
+ You'll have to have only 11 direct blocks, rather than 12, to make room for your new doubly-indirect block. 
+ You're not allowed to change the size of an on-disk inode. 
   1. The first 11 elements of ip->addrs[] should be direct blocks;
   2. The 12th should be a singly-indirect block (just like the current one); 
   3. The 13th should be your new doubly-indirect block. 
+ You are done with this exercise when bigfile writes 65803 blocks and usertests runs successfully:
```
    $ bigfile
    ..................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................................
    wrote 65803 blocks
    done; ok
    $ usertests
    ...
    ALL TESTS PASSED
    $ 
```
bigfile will take at least a minute and a half to run.

## Hints:

+ Make sure you understand `bmap()`. Draw a diagram of the relationships between `ip->addrs[]`, 
the indirect block, the doubly-indirect block and the singly-indirect blocks it points to, and data blocks. 
+ Make sure you understand why adding a doubly-indirect block increases the maximum file size 
by 256*256 blocks (really -1, since you have to decrease the number of direct blocks by one).
+ Think about how you'll index the doubly-indirect block, and the indirect blocks it points to, 
with the logical block number.
+ If you change the definition of `NDIRECT`, you'll probably have to change the declaration of `addrs[]`
 in struct inode in file.h. 
+ Make sure that struct inode and struct dinode have the same number of elements in their addrs[] arrays.
+ If you change the definition of `NDIRECT`, make sure to create a new `fs.img`, 
since mkfs uses `NDIRECT` to build the file system.
+ If your file system gets into a bad state, perhaps by crashing, delete `fs.img` (do this from Unix, not xv6).
 `make` will build a new clean file system image for you.
+ Don't forget to `brelse()` each block that you `bread()`.
+ You should allocate indirect blocks and doubly-indirect blocks only as needed, like the original `bmap()`.
+ Make sure `itrunc` frees all blocks of a file, including double-indirect blocks. 

You can run `./grade-bigfile` to see your potential grade for this task.

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
