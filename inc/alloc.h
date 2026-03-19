#ifndef ALLOC_H
#define ALLOC_H


#include <stdint.h>
#include <stddef.h>

// some heap definitions
#define HEAP_START	0x200000
#define HEAP_SIZE	0x4000
#define HEAP_END	HEAP_START+HEAP_SIZE

#define BINS		100

#define SIZE_SZ         4
#define MALLOC_ALIGN    (SIZE_SZ * 2)  // 8
#define CHNK_HDR_SZ     (SIZE_SZ * 2)  // 8

#define PREV_MASK 0x1
#define AV_MASK 0x2

#define sizenomask(s)   ((s) & ~(PREV_MASK | AV_MASK))
#define chunk2mem(c)    ((void*)((char*)(c) + CHNK_HDR_SZ))
#define mem2chunk(m)    ((chunk*)((char*)(m) - CHNK_HDR_SZ))
#define sizeallign(s)   (((s) + MALLOC_ALIGN - 1) & ~(MALLOC_ALIGN - 1))
#define chunksize(s)    ((s) + CHNK_HDR_SZ)
#define is_prev(p)      ((p)->size & PREV_MASK)
#define prev(s)         ((s) | PREV_MASK)
#define nextchunk(p)    ((chunk*)((char*)(p) + sizenomask((p)->size)))

// TAKEN FROM GLIBC https://elixir.bootlin.com/glibc/glibc-2.40/source/malloc/malloc.c#L1560
# define bin_range(sz) ((unsigned long) (sz) < (unsigned long) (BINS * (SIZE_SZ * 2)))
# define binidx(sz) (((unsigned) (sz)) >> 3)
# define bin_head(size)  arena->bins[binidx(sizenomask(size))] 


// simple chunk, single linked list.
typedef struct chunk {
    uint32_t		prevsize;
    uint32_t		size;
    struct chunk	*fd;
} chunk;


typedef struct malloc_state {
    ///// bins /////
    chunk		*bins[BINS];
    chunk		*wild;
    uint32_t		heap_size;
} malloc_state;


void *malloc(uint32_t size);
void free(void *p);



#endif
