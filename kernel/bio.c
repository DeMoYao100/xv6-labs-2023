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

#define NCACHE 10
#define NNBUF 3

struct {
  struct spinlock lock;
  struct buf buf[NNBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head;
} bcache[NCACHE];

void
binit(void)
{
  struct buf *b;
  for (int i=0;i<NCACHE;i++){
    char name[10] = {0};
    snprintf(name, 8, "bache_%d", i);
    initlock(&bcache[i].lock, name);

    // Create linked list of buffers
    bcache[i].head.prev = &bcache[i].head;
    bcache[i].head.next = &bcache[i].head;
    for(b = bcache[i].buf; b < bcache[i].buf+NNBUF; b++){
      b->next = bcache[i].head.next;
      b->prev = &bcache[i].head;
      initlock(&bcache[i].lock, "buffer");
      initsleeplock(&b->lock, name);
      bcache[i].head.next->prev = b;
      bcache[i].head.next = b;
    }
  }

}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  acquire(&bcache[blockno % NCACHE].lock);

  // Is the block already cached?
  for(b = bcache[blockno % NCACHE].head.next; b != &bcache[blockno % NCACHE].head; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache[blockno % NCACHE].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bcache[blockno % NCACHE].head.prev; b != &bcache[blockno % NCACHE].head; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache[blockno % NCACHE].lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
  // printf("lld\n");
  release(&bcache[blockno % NCACHE].lock);
  
  // no more cache on the list
  for (int i=1;i<NCACHE;i++){
    acquire(&bcache[(blockno + i) % NCACHE].lock);
      for(b = bcache[(blockno + i) % NCACHE].head.prev; b != &bcache[(blockno + i) % NCACHE].head; b = b->prev){
        if(b->refcnt == 0) {
          b->dev = dev;
          b->blockno = blockno;
          b->valid = 0;
          b->refcnt = 1;
          release(&bcache[(blockno + i) % NCACHE].lock);
          acquiresleep(&b->lock);
          return b;
        }
    }
    release(&bcache[(blockno + i) % NCACHE].lock);
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
    virtio_disk_rw(b, 0);
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
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache[b->blockno % NCACHE].lock);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache[b->blockno % NCACHE].head.next;
    b->prev = &bcache[b->blockno % NCACHE].head;
    bcache[b->blockno % NCACHE].head.next->prev = b;
    bcache[b->blockno % NCACHE].head.next = b;
  }
  
  release(&bcache[b->blockno % NCACHE].lock);
}

void
bpin(struct buf *b) {
  acquire(&bcache[b->blockno % NCACHE].lock);
  b->refcnt++;
  release(&bcache[b->blockno % NCACHE].lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache[b->blockno % NCACHE].lock);
  b->refcnt--;
  release(&bcache[b->blockno % NCACHE].lock);
}


