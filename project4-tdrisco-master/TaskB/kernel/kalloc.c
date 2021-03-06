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
} kmem;

struct {
  struct spinlock lock;
  char* mark_base;
  char* alloc_base;
} kref;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  kref.mark_base = (char*)PGROUNDUP((uint64)pa_start);
  kref.alloc_base = (char*)(kref.mark_base + 8 * PGSIZE);
  memset(kref.mark_base,1, 8*PGSIZE);


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

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP){
    panic("kfree");}

    acquire(&kref.lock);
    char r2 = --kref.mark_base[((char*)pa - kref.alloc_base)/PGSIZE];
    if(r2>0)
      {
        release(&kref.lock);
        return;
      }
    release(&kref.lock);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    {
      memset((char*)r, 5, PGSIZE); // fill with junk
      acquire(&kref.lock);
      ++kref.mark_base[((char*)r - kref.alloc_base)/PGSIZE];
      release(&kref.lock);
    }
  return (void*)r;
}

void ref_inc(void *pa)
  {
    acquire(&kref.lock);
    ++kref.mark_base[((char*)pa - kref.alloc_base)/PGSIZE];
    release(&kref.lock);
  }
