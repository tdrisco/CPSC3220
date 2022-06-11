// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock lock[3];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  struct buf head[3];
} bcache;

void
binit(void)
{
  struct buf *b;
  int i;

  initlock(&bcache.lock[0], "bcache0");
  initlock(&bcache.lock[1], "bcache1");
  initlock(&bcache.lock[2], "bcache2");


  // Create linked list of buffers
  for(i = 0; i < 3; i++)
    {
      bcache.head[i].prev = &bcache.head[i];
      bcache.head[i].next = &bcache.head[i];
    }

  i = 0;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head[i].next;
    b->prev = &bcache.head[i];
    initsleeplock(&b->lock, "buffer");
    bcache.head[i].next->prev = b;
    bcache.head[i].next = b;
    i = (i+1) % 3;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int lockNum = blockno % 3;

  acquire(&bcache.lock[lockNum]);

  // Is the block already cached?
  for(b = bcache.head[lockNum].next; b != &bcache.head[lockNum]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[lockNum]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached; recycle an unused buffer.
  for(b = bcache.head[lockNum].prev; b != &bcache.head[lockNum]; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[lockNum]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b->dev, b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b->dev, b, 1);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int lockNum = b->blockno % 3;

  acquire(&bcache.lock[lockNum]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[lockNum].next;
    b->prev = &bcache.head[lockNum];
    bcache.head[lockNum].next->prev = b;
    bcache.head[lockNum].next = b;
  }

  release(&bcache.lock[lockNum]);
}

void
bpin(struct buf *b) {

  int lockNum = b->blockno % 3;

  acquire(&bcache.lock[lockNum]);
  b->refcnt++;
  release(&bcache.lock[lockNum]);
}

void
bunpin(struct buf *b) {

  int lockNum = b->blockno % 3;

  acquire(&bcache.lock[lockNum]);
  b->refcnt--;
  release(&bcache.lock[lockNum]);
}
