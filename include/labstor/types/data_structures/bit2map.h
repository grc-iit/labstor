//
// Created by lukemartinlogan on 12/9/21.
//

#ifndef LABSTOR_BIT2MAP_H
#define LABSTOR_BIT2MAP_H

//BITMAP[BLOCK][ENTRY][BIT]

typedef uint32_t labstor_bit2map_t;

#define LABSTOR_BIT2MAP_ENTRY_SIZE_BITS 2
#define LABSTOR_BIT2MAP_BLOCK_SIZE_BITS (sizeof(labstor_bit2map_t)*8)
#define LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK (LABSTOR_BIT2MAP_BLOCK_SIZE_BITS / LABSTOR_BIT2MAP_ENTRY_SIZE_BITS)

#define LABSTOR_BIT2MAP_POS(entry_idx, entry_block)\
    labstor_bit2map_t entry_block = (entry_idx) / LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK;\
    uint8_t entry_in_block = (entry_idx) % LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK;\
    uint8_t entry_start = entry_in_block * LABSTOR_BIT2MAP_ENTRY_SIZE_BITS;

static inline uint32_t labstor_bit2map_GetSize(int num_entries) {
    if(num_entries % LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK == 0) {
        return (num_entries / LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK) * sizeof(labstor_bit2map_t);
    } else {
        return (num_entries / LABSTOR_BIT2MAP_ENTRIES_PER_BLOCK + 1) * sizeof(labstor_bit2map_t);
    }
}

static inline void* labstor_bit2map_GetNextSection(labstor_bit2map_t *bitmap, uint32_t num_entries) {
    return (void*)((char*)bitmap + labstor_bit2map_GetSize(num_entries));
}

static inline void labstor_bit2map_Init(labstor_bit2map_t *bitmap, uint32_t num_entries) {
    memset(bitmap, 0, labstor_bit2map_GetSize(num_entries));
}

static inline void labstor_bit2map_Set(labstor_bit2map_t *bitmap, uint32_t entry_idx, labstor_bit2map_t mask) {
    LABSTOR_BIT2MAP_POS(entry_idx, entry_block)
    mask <<= entry_start;
    __atomic_fetch_or(&bitmap[entry_block], mask, __ATOMIC_RELAXED);
}

static inline void labstor_bit2map_Xor(labstor_bit2map_t *bitmap, uint32_t entry_idx, labstor_bit2map_t mask) {
    LABSTOR_BIT2MAP_POS(entry_idx, entry_block)
    mask <<= entry_start;
    __atomic_fetch_xor(&bitmap[entry_block], mask, __ATOMIC_RELAXED);
}

static inline int labstor_bit2map_TestAndSet(labstor_bit2map_t *bitmap, uint32_t entry_idx, labstor_bit2map_t test, labstor_bit2map_t set) {
    LABSTOR_BIT2MAP_POS(entry_idx, entry_block)
    labstor_bit2map_t tmp, block;
    set <<= entry_start;
    test <<= entry_start;
    do {
        tmp = bitmap[entry_block];
        block = tmp | set;
        if (tmp & test) return false;
    }
    while(!__atomic_compare_exchange_n(
                &bitmap[entry_block],
                &tmp,
                block, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline void labstor_bit2map_Unset(labstor_bit2map_t *bitmap, uint32_t entry_idx, labstor_bit2map_t mask) {
    LABSTOR_BIT2MAP_POS(entry_idx, entry_block)
    mask = ~(mask << entry_start);
    __atomic_fetch_and(&bitmap[entry_block], mask, __ATOMIC_RELAXED);
}

static inline bool labstor_bit2map_IsSet(labstor_bit2map_t *bitmap, uint32_t entry_idx, labstor_bit2map_t mask) {
    LABSTOR_BIT2MAP_POS(entry_idx, entry_block)
    mask = (mask << entry_start);
    return (bitmap[entry_block] & mask) != 0;
}

#endif //LABSTOR_BIT2MAP_H
