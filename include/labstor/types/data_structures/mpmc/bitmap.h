//
// Created by lukemartinlogan on 12/9/21.
//

#ifndef LABSTOR_BITMAP_H
#define LABSTOR_BITMAP_H

typedef uint32_t labstor_bitmap_t;

#define LABSTOR_BITMAP_BLOCK_SIZE_BITS (sizeof(labstor_bitmap_t)*8)
#define LABSTOR_BITMAP_ENTRIES_PER_BLOCK (sizeof(labstor_bitmap_t)*8)

#define LABSTOR_BITMAP_POS(entry_idx, entry_block, bit_in_block)\
    labstor_bitmap_t entry_block = entry_idx / LABSTOR_BITMAP_BLOCK_SIZE_BITS;\
    uint8_t bit_in_block = entry_idx % LABSTOR_BITMAP_BLOCK_SIZE_BITS;

static inline uint32_t labstor_bitmap_GetSize(int num_entries) {
    if(num_entries % LABSTOR_BITMAP_ENTRIES_PER_BLOCK == 0) {
        return (num_entries / LABSTOR_BITMAP_ENTRIES_PER_BLOCK) * sizeof(labstor_bitmap_t);
    } else {
        return (num_entries / LABSTOR_BITMAP_ENTRIES_PER_BLOCK + 1) * sizeof(labstor_bitmap_t);
    }
}

static inline void* labstor_bitmap_GetNextSection(labstor_bitmap_t *bitmap, uint32_t num_entries) {
    return (void*)((char*)bitmap + labstor_bitmap_GetSize(num_entries));
}

static inline void labstor_bitmap_Init(labstor_bitmap_t *bitmap, uint32_t num_entries) {
    memset(bitmap, 0, labstor_bitmap_GetSize(num_entries));
}

static inline void labstor_bitmap_Set(labstor_bitmap_t *bitmap, uint32_t entry_idx) {
    LABSTOR_BITMAP_POS(entry_idx, entry_block, bit_in_block)
    labstor_bitmap_t mask = (labstor_bitmap_t)1 << bit_in_block;
    __atomic_fetch_or(&bitmap[entry_block], mask, __ATOMIC_RELAXED);
}

static inline bool labstor_bitmap_TestAndSet(labstor_bitmap_t *bitmap, uint32_t entry_idx) {
    LABSTOR_BITMAP_POS(entry_idx, entry_block, bit_in_block)
    labstor_bitmap_t mask = (labstor_bitmap_t)1 << bit_in_block;
    labstor_bitmap_t block = bitmap[entry_block];
    return __atomic_compare_exchange_n(
            &bitmap[entry_block],
            &block,
            block | mask, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
}

static inline void labstor_bitmap_Unset(labstor_bitmap_t *bitmap, uint32_t entry_idx) {
    LABSTOR_BITMAP_POS(entry_idx, entry_block, bit_in_block)
    labstor_bitmap_t mask = ~((labstor_bitmap_t)1 << bit_in_block);
    __atomic_fetch_and(&bitmap[entry_block], mask, __ATOMIC_RELAXED);
}

static inline bool labstor_bitmap_IsSet(labstor_bitmap_t *bitmap, uint32_t entry_idx) {
    LABSTOR_BITMAP_POS(entry_idx, entry_block, bit_in_block)
    labstor_bitmap_t mask = ((labstor_bitmap_t)1 << bit_in_block);
    return (bitmap[entry_block] & mask) != 0;
}

#endif //LABSTOR_BITMAP_H
