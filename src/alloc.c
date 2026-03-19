#include "../inc/bootix.h"

bool heapinit = false;
malloc_state *arena = (malloc_state *) HEAP_START;

static void heap_init() {
	if (!heapinit) {
		arena = (malloc_state *) HEAP_START;
		for (int i = 0; i < BINS; i++) arena->bins[i] = NULL;

		arena->wild = (chunk *)(HEAP_START + sizeof(malloc_state));
		arena->heap_size = HEAP_SIZE - sizeof(malloc_state);
		arena->wild->prevsize = 0; // not used
		arena->wild->size = arena->heap_size | PREV_MASK; 
		arena->wild->fd = NULL;
		heapinit = true;
	}
}

// a simple heap implimentation close to glibc heap
void *malloc(uint32_t size) {
	uint32_t asize;
	chunk *victim;
	uint32_t total;

	if (size == 0) return NULL;
	heap_init();

	asize = sizeallign(size);
	total = asize + CHNK_HDR_SZ; 

	if (bin_range(asize)) {
		int idx = binidx(asize);
		if (arena->bins[idx]) {
			victim = arena->bins[idx];
			arena->bins[idx] = victim->fd;
			chunk *next = (chunk *)((char*)victim + sizenomask(victim->size));
			next->size |= PREV_MASK;
			return chunk2mem(victim);
		}
	}

	if (total <= sizenomask(arena->wild->size)) {
		victim = arena->wild;
		arena->wild = (chunk *)((char*)arena->wild + total);
		arena->wild->prevsize = total; 
		arena->wild->size = (sizenomask(victim->size) - total) | PREV_MASK;
		arena->wild->fd = NULL;

		victim->size = total; 
		uint32_t old_flags = victim->size & (PREV_MASK | AV_MASK);
		victim->size = total | old_flags;
		return chunk2mem(victim);
	}
	
	return NULL;
}

void free(void *p) {
	if (p == NULL) return;
	chunk *victim = mem2chunk(p);
	uint32_t victim_size = sizenomask(victim->size);
	chunk *next = (chunk *)((char*)victim + victim_size);

	if (next == arena->wild) {
		victim->size = victim_size + sizenomask(arena->wild->size);
		arena->wild = victim;
		return;
	}
	
	if (!(victim->size & PREV_MASK)) {
		uint32_t prev_size = victim->prevsize;
		chunk *prev = (chunk *)((char*)victim - prev_size);
		(void) prev;
		(void) prev_size;
		// merge later idk ??
	}

	uint32_t asize = victim_size - CHNK_HDR_SZ;
	if (bin_range(asize)) {
		int idx = binidx(asize);
		victim->fd = arena->bins[idx];
		arena->bins[idx] = victim;
		next->size &= ~PREV_MASK;
		next->prevsize = victim_size;
	}
}
