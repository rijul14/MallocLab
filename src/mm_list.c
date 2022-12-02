#include <mm_list.h>  // prototypes of functions implemented in this file
#include <unistd.h>   // NULL

BlockHeader *mm_list_headp;
BlockHeader *mm_list_tailp;

/**
 * Initializes to an empty list.
 */
void mm_list_init() {
    mm_list_headp = NULL;
    mm_list_tailp = NULL;
}

/**
 * In addition to the block header with size/allocated bit, a free block has
 * pointers to the headers of the previous and next blocks on the free list.
 *
 * Pointers use 4 bytes because this project is compiled with -m32.
 * Check Figure 9.48(b) in the textbook.
 */
typedef struct {
    BlockHeader header;
    BlockHeader *prev_free;
    BlockHeader *next_free;
} FreeBlockHeader;

/**
 * Find the header address of the previous **free** block on the **free list**.
 *
 * @param bp address of a block header (it must be a free block)
 * @return address of the header of the previous free block on the list
 */
BlockHeader *mm_list_prev(BlockHeader *bp) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    return fp->prev_free;
}

/**
 * Find the header address of the next **free** block on the **free list**.
 *
 * @param bp address of a block header (it must be a free block)
 * @return address of the header of the next free block on the list
 */
BlockHeader *mm_list_next(BlockHeader *bp) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    return fp->next_free;
}

/**
 * Set the pointer to the previous **free** block.
 *
 * @param bp address of a free block header
 * @param prev address of the header of the previous free block (to be set)
 */
static void mm_list_prev_set(BlockHeader *bp, BlockHeader *prev) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    fp->prev_free = prev;
}


/**
 * Set the pointer to the next **free** block.
 *
 * @param bp address of a free block header
 * @param next address of the header of the next free block (to be set)
 */
static void mm_list_next_set(BlockHeader *bp, BlockHeader *next) {
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    fp->next_free = next;
}

/**
 * Add a block at the beginning of the free list.
 *
 * @param bp address of the header of the block to add
 */
void mm_list_prepend(BlockHeader *bp) {
    // TODO: implement
    if (mm_list_headp == NULL) {
        FreeBlockHeader *fp = (FreeBlockHeader *)bp;
        fp->next_free = NULL;
        fp->prev_free = NULL;
        mm_list_headp = bp;
        mm_list_tailp = bp;
    }
    else {
        FreeBlockHeader *fp = (FreeBlockHeader *)bp;
        FreeBlockHeader *h = (FreeBlockHeader *)mm_list_headp;
        fp->next_free = mm_list_headp;
        fp->prev_free = NULL;
        h->prev_free = bp;
        mm_list_headp = bp;
    }
}

/**
 * Add a block at the end of the free list.
 *
 * @param bp address of the header of the block to add
 */
void mm_list_append(BlockHeader *bp) {
    // TODO: implement
    if (mm_list_headp == NULL) {
        FreeBlockHeader *fp = (FreeBlockHeader *)bp;
        fp->next_free = NULL;
        fp->prev_free = NULL;
        mm_list_headp = bp;
        mm_list_tailp = bp;
    }
    else {
        FreeBlockHeader *t = (FreeBlockHeader *)mm_list_tailp;
        FreeBlockHeader *fp = (FreeBlockHeader *)bp;
        t->next_free = bp;
        fp->prev_free = mm_list_tailp;
        fp->next_free = NULL;
        mm_list_tailp = bp;
    }
}

/**
 * Remove a block from the free list.
 *
 * @param bp address of the header of the block to remove
 */
void mm_list_remove(BlockHeader *bp) {
    // TODO: implement
    if (mm_list_headp == NULL) {
        return;
    }
    FreeBlockHeader *fp = (FreeBlockHeader *)bp;
    if (mm_list_headp == bp) {
        mm_list_headp = fp->next_free;
    }
    if (mm_list_tailp == bp) {
        mm_list_tailp = fp->prev_free;
    }
    if (fp->next_free != NULL) {
        mm_list_prev_set(fp->next_free,fp->prev_free);
    }
    if (fp->prev_free != NULL) {
        mm_list_next_set(fp->prev_free,fp->next_free);
    }
}
