
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

#ifndef LABSTOR_BLOCK_ALLOCATOR_H
#define LABSTOR_BLOCK_ALLOCATOR_H

#include <vector>
#include <cstdint>
#include "labstor/types/data_structures/shmem_ring_buffer.h"

#define SMALL_BLOCK_SIZE (4*(1<<10))
#define LARGE_BLOCK_SIZE (128*(1<<10))

namespace labstor::LabFS {

struct Block {
    size_t off_;
    int size_;
    Block() = default;
    Block(size_t off, int size) : off_(off), size_(size) {}
};

class BlockAllocator {
private:
    labstor::ipc::mpmc::ring_buffer<Block> small_blocks_;
    labstor::ipc::mpmc::ring_buffer<Block> large_blocks_;
public:
    static size_t GetNumLargeBlocks(size_t disk_size, size_t num_small_blocks) {
        return (disk_size - num_small_blocks*SMALL_BLOCK_SIZE)/LARGE_BLOCK_SIZE;;
    }

    static size_t GetSize(size_t disk_size, size_t num_small_blocks) {
        size_t small_block_size = labstor::ipc::mpmc::ring_buffer<Block>::GetSize(num_small_blocks);
        size_t large_block_size = labstor::ipc::mpmc::ring_buffer<Block>::GetSize(GetNumLargeBlocks(disk_size, num_small_blocks));
        return small_block_size + large_block_size;
    }

    void Initialize(size_t disk_off, size_t disk_size, size_t num_small_blocks, void *region, size_t region_size) {
        size_t off = disk_off;
        size_t num_large_blocks = GetNumLargeBlocks(disk_size, num_small_blocks);
        void *section = region;
        size_t section_size = region_size;

        //Small blocks
        small_blocks_.Init(section, section_size, num_small_blocks);
        for(int i = 0; i < num_small_blocks; ++i) {
            small_blocks_.Enqueue(Block(off, SMALL_BLOCK_SIZE));
            off += SMALL_BLOCK_SIZE;
        }
        section = small_blocks_.GetNextSection();
        section_size -= small_blocks_.GetSize();

        //Large blocks
        large_blocks_.Init(section, section_size, num_large_blocks);
        for(int i = 0; i < num_large_blocks; ++i) {
            large_blocks_.Enqueue(Block(off, LARGE_BLOCK_SIZE));
            off += LARGE_BLOCK_SIZE;
        }
    }

    void Attach(void *region) {
        void *section = region;
        small_blocks_.Attach(section);
        section = small_blocks_.GetNextSection();
        large_blocks_.Attach(section);
    }

    void GetBlock(int size, Block &block) {
        if(size == SMALL_BLOCK_SIZE) {
            small_blocks_.Dequeue(block);
        } else {
            large_blocks_.Dequeue(block);
        }
    }

    void FreeBlock(Block &block) {
        if(block.size_ == SMALL_BLOCK_SIZE) {
            small_blocks_.Enqueue(block);
        } else {
            large_blocks_.Enqueue(block);
        }
    }
};

}

#endif //LABSTOR_BLOCK_ALLOCATOR_H