// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  char lockNames[6];
} kmem[NCPU];

void
kinit()
{
  int i;
  char tempName[5];
  strncpy(tempName,"kmem",5);
  for(i = 0; i<NCPU;i++)
    {
      tempName[4] = '0' +i;
      strncpy(kmem[i].lockNames,tempName,6);
      initlock(&kmem[i].lock, kmem[i].lockNames);
    }

  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  int currentCore = cpuid();
  acquire(&kmem[currentCore].lock);
  r->next = kmem[currentCore].freelist;
  kmem[currentCore].freelist = r;
  release(&kmem[currentCore].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int currentCore = cpuid();
  acquire(&kmem[currentCore].lock);
  r = kmem[currentCore].freelist;
  if(r!= 0)
    {
      kmem[currentCore].freelist = r->next;
      release(&kmem[currentCore].lock);
      pop_off();

      memset((char*)r, 5, PGSIZE); // fill with junk
      return (void*)r;
    }
  else
    {
      release(&kmem[currentCore].lock);

      int i;
      for(i = (currentCore+1) % NCPU; i != currentCore; i = (i+1) % NCPU)
        {
          acquire(&kmem[i].lock);
          r = kmem[i].freelist;
          if(r)
            break;
          release(&kmem[i].lock);
        }
      if(currentCore == i)
        {
          pop_off();
          return 0;
        }
      kmem[i].freelist = r->next;
      release(&kmem[i].lock);
      pop_off();
      memset((char*)r, 5, PGSIZE); // fill with junk
      return (void*)r;
    }


  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
