
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef LABSTOR_BIT2MAP_H
#define LABSTOR_BIT2MAP_H

//BITMAP[BLOCK][ENTRY][BIT]

#define LABSTOR_BIT2MAP_VALID 1
#define LABSTOR_BIT2MAP_BEING_SET 2

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

static inline bool labstor_bit2map_IsSet(labstor_bit2map_t *bitmap, uint32_t entry_idx, labstor_bit2map_t mask) {
    LABSTOR_BIT2MAP_POS(entry_idx, entry_block)
    mask = (mask << entry_start);
    return (bitmap[entry_block] & mask) != 0;
}

static inline bool labstor_bit2map_IsValid(labstor_bit2map_t *bitmap, uint32_t entry_idx) {
    return labstor_bit2map_IsSet(bitmap, entry_idx, LABSTOR_BIT2MAP_VALID);
}

static inline bool labstor_bit2map_IsBeingSet(labstor_bit2map_t *bitmap, uint32_t entry_idx) {
    return labstor_bit2map_IsSet(bitmap, entry_idx, LABSTOR_BIT2MAP_BEING_SET);
}

static inline int labstor_bit2map_BeginModify(labstor_bit2map_t *bitmap, uint32_t entry_idx) {
    //Test if VALID & BEING_SET bits are not set
    //Mark the entry as BEING_SET
    labstor_bit2map_t tmp, block, valid_or_set, being_set;
    LABSTOR_BIT2MAP_POS(entry_idx, entry_block)
    valid_or_set = (LABSTOR_BIT2MAP_VALID | LABSTOR_BIT2MAP_BEING_SET) << entry_start;
    being_set = (LABSTOR_BIT2MAP_BEING_SET) << entry_start;
    do {
        //Get current block
        tmp = bitmap[entry_block];
        //Verify that the block is not VALID or SET
        if (tmp & valid_or_set) return false;
        //Set block as BEING_SET
        block = (tmp | being_set);
    }
    //Check if the block has been updated
    while(!__atomic_compare_exchange_n(
                &bitmap[entry_block],
                &tmp,
                block, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline void labstor_bit2map_CommitModify(labstor_bit2map_t *bitmap, uint32_t entry_idx) {
    //Assumes BEING_SET, but not VALID
    //Marks the entry as VALID, but not BEING_SET
    labstor_bit2map_t valid_mask, not_being_set_mask, tmp, block;
    LABSTOR_BIT2MAP_POS(entry_idx, entry_block)
    valid_mask = LABSTOR_BIT2MAP_VALID << entry_start;
    not_being_set_mask = ~(LABSTOR_BIT2MAP_BEING_SET << entry_start);
    do {
        //Get current block
        tmp = bitmap[entry_block];
        //Set block as VALID & not BEING_SET
        block = (tmp | valid_mask) & not_being_set_mask;
    }
    //Check that the block didn't get modified
    while(!__atomic_compare_exchange_n(
            &bitmap[entry_block],
            &tmp,
            block, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
}

static inline bool labstor_bit2map_BeginRemove(labstor_bit2map_t *bitmap, uint32_t entry_idx) {
    //Check if VALID, but not BEING_SET
    //Marks the entry as VALID and BEING_SET
    labstor_bit2map_t tmp, block, valid_and_set, test_valid, test_being_set;
    LABSTOR_BIT2MAP_POS(entry_idx, entry_block)
    test_valid = (LABSTOR_BIT2MAP_VALID) << entry_start;
    test_being_set = (LABSTOR_BIT2MAP_BEING_SET) << entry_start;
    valid_and_set = (LABSTOR_BIT2MAP_VALID | LABSTOR_BIT2MAP_BEING_SET) << entry_start;
    do {
        tmp = bitmap[entry_block];
        block = (tmp | valid_and_set);
        if (!(tmp & test_valid) || (tmp & test_being_set)) { return false; }
    }
    while(!__atomic_compare_exchange_n(
            &bitmap[entry_block],
            &tmp,
            block, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
    return true;
}

static inline void labstor_bit2map_CommitRemove(labstor_bit2map_t *bitmap, uint32_t entry_idx) {
    //Assumes the entry is VALID and BEING_SET
    //Marks the entry as neither VALID nor BEING_SET
    uint32_t mask;
    LABSTOR_BIT2MAP_POS(entry_idx, entry_block)
    mask = ~((LABSTOR_BIT2MAP_BEING_SET | LABSTOR_BIT2MAP_VALID) << entry_start);
    __atomic_fetch_and(&bitmap[entry_block], mask, __ATOMIC_RELAXED);
}

static inline void labstor_bit2map_IgnoreRemove(labstor_bit2map_t *bitmap, uint32_t entry_idx) {
    labstor_bit2map_CommitModify(bitmap, entry_idx);
}

static inline void labstor_bit2map_Unset(labstor_bit2map_t *bitmap, uint32_t entry_idx, labstor_bit2map_t mask) {
    LABSTOR_BIT2MAP_POS(entry_idx, entry_block)
    mask = ~(mask << entry_start);
    __atomic_fetch_and(&bitmap[entry_block], mask, __ATOMIC_RELAXED);
}

#endif //LABSTOR_BIT2MAP_H