//
// Created by lukemartinlogan on 12/9/21.
//

#ifndef LABSTOR_hexmap_H
#define LABSTOR_hexmap_H

typedef uint8_t labstor_hexmap_t;

#define LABSTOR_HEXMAP_SIZE_BITS (sizeof(labstor_hexmap_t)*8)
#define LABSTOR_HEXMAP_SIZE_HEXS (LABSTOR_HEXMAP_SIZE_BITS/4)

#define LABSTOR_HEXMAP_POS(hex_idx, hex_bit, hexblock, bit_in_block)\
    labstor_hexmap_t hexblock = hex_idx / LABSTOR_HEXMAP_SIZE_HEXS;\
    uint8_t hex_in_block = hex_idx % LABSTOR_HEXMAP_SIZE_HEXS;\
    uint8_t bit_in_block = hex_bit + hex_in_block * 4;

static inline uint32_t labstor_hexmap_GetSize(int num_entries) {
    if(num_entries % LABSTOR_HEXMAP_SIZE_BITS == 0) {
        return num_entries / LABSTOR_HEXMAP_SIZE_BITS;
    } else {
        return num_entries / LABSTOR_HEXMAP_SIZE_BITS + 1;
    }
}

static inline void* labstor_hexmap_GetNextSection(labstor_hexmap_t *hexmap, uint32_t num_entries) {
    return (void*)((char*)hexmap + labstor_hexmap_GetSize(num_entries));
}

static inline void labstor_hexmap_Init(labstor_hexmap_t *hexmap, uint32_t num_entries) {
    memset(hexmap, 0, labstor_hexmap_GetSize(num_entries));
}

static inline void labstor_hexmap_Set(labstor_hexmap_t *hexmap, uint32_t hex_idx, uint8_t hexbit) {
    LABSTOR_HEXMAP_POS(hex_idx, hexbit, hexblock, bit_in_block)
    labstor_hexmap_t mask = (labstor_hexmap_t)1 << bit_in_block;
    __atomic_fetch_or(&hexmap[bitblock], mask, __ATOMIC_RELAXED);
}

static inline void labstor_hexmap_Unset(labstor_hexmap_t *hexmap, uint32_t hex_idx, uint8_t hexbit) {
    LABSTOR_HEXMAP_POS(hex_idx, hexbit, hexblock, bit_in_block)
    labstor_hexmap_t mask = ~((labstor_hexmap_t)1 << bit_in_block);
    __atomic_fetch_and(&hexmap[bitblock], mask, __ATOMIC_RELAXED);
}

static inline bool labstor_hexmap_IsSet(labstor_hexmap_t *hexmap, uint32_t hex_idx, uint8_t hexbit) {
    LABSTOR_HEXMAP_POS(hex_idx, hexbit, hexblock, bit_in_block)
    labstor_hexmap_t mask = ((labstor_hexmap_t)1 << bit_in_block);
    return (hexmap[bitblock] & mask) != 0;
}

#endif //LABSTOR_hexmap_H
