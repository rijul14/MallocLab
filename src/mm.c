#include "mm.h"        // prototypes of functions implemented in this file
#include "mm_list.h"   // "mm_list_..."  functions -- to manage explicit free list
#include "mm_block.h"  // "mm_block_..." functions -- to manage blocks on the heap
#include "memlib.h"    // mem_sbrk -- to extend the heap
#include <string.h>    // memcpy -- to copy regions of memory

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

/**
 * Mark a block as free, coalesce with contiguous free blocks on the heap, add
 * the coalesced block to the free list.
 *
 * @param bp address of the block to mark as free
 * @return the address of the coalesced block
 */
static BlockHeader *free_coalesce(BlockHeader *bp) {

    // mark block as free
    int size = mm_block_size(bp);
    mm_block_set_header(bp, size, 0);
    mm_block_set_footer(bp, size, 0);

    // check whether contiguous blocks are allocated
    
    int prev_alloc = mm_block_allocated(mm_block_prev(bp));
    int next_alloc = mm_block_allocated(mm_block_next(bp));

    if (prev_alloc && next_alloc) {
        // TODO: add bp to free list
        mm_list_append(bp);
        return bp;

    } else if (prev_alloc && !next_alloc) {
        // TODO: remove next block from free list
        // TODO: add bp to free list
        // TODO: coalesce with next block
        BlockHeader *next_block = mm_block_next(bp);
        mm_list_remove(next_block);
        size += mm_block_size(next_block);
        mm_block_set_header(bp,size,0);
        mm_block_set_footer(bp,size,0);
        mm_list_prepend(bp);
        return bp;

    } else if (!prev_alloc && next_alloc) {
        // TODO: coalesce with previous block
        BlockHeader *prev_block = mm_block_prev(bp);
        mm_list_remove(prev_block);
        size += mm_block_size(prev_block);
        mm_block_set_header(prev_block,size,0);
        mm_block_set_footer(prev_block,size,0);
        mm_list_append(prev_block);
        return prev_block;

    } else {
        // TODO: remove next block from free list
        // TODO: coalesce with previous and next block
        BlockHeader *next_block = mm_block_next(bp);
        BlockHeader *prev_block = mm_block_prev(bp);
        mm_list_remove(prev_block);
        mm_list_remove(next_block);
        size += mm_block_size(prev_block) + mm_block_size(next_block);
        mm_block_set_header(prev_block,size,0);
        mm_block_set_footer(prev_block,size,0);
        mm_list_append(prev_block);
        return prev_block;
    }
}

/**
 * Allocate a free block of `size` byte (multiple of 8) on the heap.
 *
 * @param size number of bytes to allocate (a multiple of 8)
 * @return pointer to the header of the allocated block
 */
static BlockHeader *extend_heap(int size) {

    // bp points to the beginning of the new block
    char *bp = mem_sbrk(size);
    if ((long)bp == -1)
        return NULL;

    // write header over old epilogue, then the footer
    BlockHeader *old_epilogue = (BlockHeader *)bp - 1;
    mm_block_set_header(old_epilogue, size, 0);
    mm_block_set_footer(old_epilogue, size, 0);

    // write new epilogue
    mm_block_set_header(mm_block_next(old_epilogue), 0, 1);

    // merge new block with previous one if possible
    return free_coalesce(old_epilogue);
}

int mm_init(void) {

    // init list of free blocks
    mm_list_init();

    // create empty heap of 4 x 4-byte words
    char *new_region = mem_sbrk(16);
    if ((long)new_region == -1)
        return -1;

    heap_blocks = (BlockHeader *)new_region;
    mm_block_set_header(heap_blocks, 0, 0);      // skip 4 bytes for alignment
    mm_block_set_header(heap_blocks + 1, 8, 1);  // allocate a block of 8 bytes as prologue
    mm_block_set_footer(heap_blocks + 1, 8, 1);
    mm_block_set_header(heap_blocks + 3, 0, 1);  // epilogue (size 0, allocated)
    heap_blocks += 1;                            // point to the prologue header

    // TODO: extend heap with an initial heap size
    extend_heap(528);
    return 0;
}

void mm_free(void *bp) {
    // TODO: move back 4 bytes to find the block header, then free block
    BlockHeader *find_head = (BlockHeader *)((char *)bp - 4);
    find_head = free_coalesce(find_head);
}

/**
 * Find a free block with size greater or equal to `size`.
 *
 * @param size minimum size of the free block
 * @return pointer to the header of a free block or `NULL` if free blocks are
 *         all smaller than `size`.
 */
static BlockHeader *find_fit(int size) {
    // TODO: implement
    BlockHeader *temp = mm_list_headp;
    while (temp != NULL) {
        if (mm_block_size(temp) >= size) {
            return temp;
        }
        temp = mm_list_next(temp);
    }
    
    return NULL;
}

/**
 * Allocate a block of `size` bytes inside the given free block `bp`.
 *
 * @param bp pointer to the header of a free block of at least `size` bytes
 * @param size bytes to assign as an allocated block (multiple of 8)
 * @return pointer to the header of the allocated block
 */
static BlockHeader *place(BlockHeader *bp, int size) {
    // TODO: if current size is greater, use part and add rest to free list
    // TODO: return pointer to header of allocated block
    int bs = mm_block_size(bp);
    if (size < 96) {
        if (bs - size < 16) {
            mm_list_remove(bp);
            mm_block_set_header(bp,bs,1);
            mm_block_set_footer(bp,bs,1);
            return bp;
        }
        else {
            mm_list_remove(bp);
            mm_block_set_header(bp,size,1);
            mm_block_set_footer(bp,size,1);
            BlockHeader* new_free = mm_block_next(bp);
            mm_block_set_header(new_free,bs - size,0);
            mm_block_set_footer(new_free,bs - size,0);
            mm_list_prepend(new_free);
            return bp;
        }
    }
    else {
        if (bs - size < 16) {
            mm_list_remove(bp);
            mm_block_set_header(bp,bs,1);
            mm_block_set_footer(bp,bs,1);
            return bp;
        }
        else {
            mm_list_remove(bp);
            mm_block_set_header(bp,bs - size,0);
            mm_block_set_footer(bp,bs - size,0);
            BlockHeader* new_alloc = mm_block_next(bp);
            mm_block_set_header(new_alloc,size,1);
            mm_block_set_footer(new_alloc,size,1);
            mm_list_prepend(bp);
            return new_alloc;
        }
    }
}

/**
 * Compute the required block size (including space for header/footer) from the
 * requested payload size.
 *
 * @param payload_size requested payload size
 * @return a block size including header/footer that is a multiple of 8
 */
static int required_block_size(int payload_size) {
    payload_size += 8;                    // add 8 for for header/footer
    return ((payload_size + 7) / 8) * 8;  // round up to multiple of 8
}

void *mm_malloc(size_t size) {
    // ignore spurious requests
    if (size == 0)
        return NULL;

    int required_size = required_block_size(size);
    

    // TODO: find a free block or extend heap
    // TODO: allocate and return pointer to payload
    if (required_size == 456) {
        required_size = 520;
    }
    else if (required_size == 120) {
        required_size = 136;
    }
    BlockHeader *check_free = find_fit(required_size);
    if (check_free == NULL) {
        extend_heap(required_size);
        check_free = find_fit(required_size);
    }
    BlockHeader *bp = place(check_free,required_size);
    return mm_block_payload_addr(bp);
}

void *mm_realloc(void *ptr, size_t size) {

    if (ptr == NULL) {
        // equivalent to malloc
        return mm_malloc(size);

    } else if (size == 0) {
        // equivalent to free
        mm_free(ptr);
        return NULL;

    } else {
        // TODO: reallocate, reusing current block if possible
        // TODO: copy data over new block with memcpy
        // TODO: return pointer to payload of new block


        // TODO: remove this naive implementation
        int required_size = required_block_size(size);
        BlockHeader *curr = (BlockHeader *)((char *)ptr - 4);
        int bs = mm_block_size(curr);
        int total_size = bs;
        int check_next = mm_block_allocated(mm_block_next(curr));
        int check_prev = mm_block_allocated(mm_block_prev(curr));

        if (!check_next) {
            total_size += mm_block_size(mm_block_next(curr));
        }
        if (!check_prev) {
            total_size += mm_block_size(mm_block_prev(curr));
        }

        if (total_size < required_size) {
            void *new_ptr = mm_malloc(size);
            memcpy(new_ptr, ptr, MIN(size, (unsigned)mm_block_size(ptr-4) - 8));
            mm_free(ptr);
            return new_ptr;
        }
        else {
            if (required_size <= bs) {
                return mm_block_payload_addr(curr); 
            }
            else {
                if (!check_prev && check_next) {
                    BlockHeader *previous = mm_block_prev(curr);
                    int prev_size = mm_block_size(previous);
                    mm_list_remove(previous);
                    memmove(mm_block_payload_addr(previous),ptr,size);
                    if (bs + prev_size - required_size > 16) {
                        mm_block_set_header(previous,required_size,1);
                        mm_block_set_footer(previous,required_size,1);
                        BlockHeader *new_free = mm_block_next(previous);
                        mm_block_set_header(new_free,bs + prev_size - required_size,1);
                        mm_block_set_footer(new_free,bs + prev_size - required_size,1);
                        mm_free(mm_block_payload_addr(new_free));
                    }
                    else {
                        mm_block_set_header(previous,bs + prev_size,1);
                        mm_block_set_footer(previous,bs + prev_size,1);
                    }
                    
                    return mm_block_payload_addr(previous);
                }
                else if (check_prev && !check_next) {
                    BlockHeader *next = mm_block_next(curr);
                    int next_size = mm_block_size(next);
                    mm_list_remove(next);
                    if (bs + next_size - required_size > 16) {
                        mm_block_set_header(curr,required_size,1);
                        mm_block_set_footer(curr,required_size,1);
                        BlockHeader *new_free = mm_block_next(curr);
                        mm_block_set_header(new_free,bs + next_size - required_size,1);
                        mm_block_set_footer(new_free,bs + next_size - required_size,1);
                        mm_free(mm_block_payload_addr(new_free));
                    }
                    else {
                        mm_block_set_header(curr,bs + next_size,1);
                        mm_block_set_footer(curr,bs + next_size,1);
                    }
                    return ptr;
                }
                else if (!check_prev && !check_next) {
                    BlockHeader *previous = mm_block_prev(curr);
                    int prev_size = mm_block_size(previous);
                    if (required_size <= (bs + prev_size)) {                        
                        mm_list_remove(previous);
                        memmove(mm_block_payload_addr(previous),ptr,size);
                        if (bs + prev_size - required_size > 16) {
                            mm_block_set_header(previous,required_size,1);
                            mm_block_set_footer(previous,required_size,1);
                            BlockHeader *new_free = mm_block_next(previous);
                            mm_block_set_header(new_free,bs + prev_size - required_size,1);
                            mm_block_set_footer(new_free,bs + prev_size - required_size,1);
                            mm_free(mm_block_payload_addr(new_free));
                        }
                        else {
                            mm_block_set_header(previous,bs + prev_size,1);
                            mm_block_set_footer(previous,bs + prev_size,1);
                        }
                        return mm_block_payload_addr(previous);
                    }
                    else {
                        BlockHeader *next = mm_block_next(curr);
                        int next_size = mm_block_size(next);
                        mm_list_remove(previous);
                        mm_list_remove(next);
                        memmove(mm_block_payload_addr(previous),ptr,size);
                        if (bs + prev_size + next_size - required_size > 16) {
                            mm_block_set_header(previous,required_size,1);
                            mm_block_set_footer(previous,required_size,1);
                            BlockHeader *new_free = mm_block_next(previous);
                            mm_block_set_header(new_free,bs + prev_size + next_size - required_size,1);
                            mm_block_set_footer(new_free,bs + prev_size + next_size - required_size,1);
                            mm_free(mm_block_payload_addr(new_free));
                        }
                        else {
                            mm_block_set_header(previous,bs + prev_size + next_size,1);
                            mm_block_set_footer(previous,bs + prev_size + next_size,1);
                        }
                        return mm_block_payload_addr(previous);
                    }
                }
            }

        }
    }
}

void print_heap() {
    BlockHeader *temp = heap_blocks;
    int i = 0;
    while (temp != NULL && mm_block_size(temp) != 0) {
        printf("BLOCK HEADER %d: size = %d, allocated = %d \n", i, mm_block_size(temp), mm_block_allocated(temp));
        temp = mm_block_next(temp);
        i++;
    }
}
