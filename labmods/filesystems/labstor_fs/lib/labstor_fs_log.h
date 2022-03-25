//
// Created by lukemartinlogan on 3/13/22.
//

#ifndef LABSTOR_LABSTOR_FS_LOG_H
#define LABSTOR_LABSTOR_FS_LOG_H

/*
 * We have a device with 1TB of data; how do we divide blocks on the storage device?
 * First 64MB is a log, per-say
 * */

#include <vector>
#include <list>
#include <labmods/kernel/secure_shmem/netlink_client/secure_shmem_client_netlink.h>
#include <labstor/types/allocator/allocator.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include "block_allocator.h"

namespace labstor::LabFS {

struct LogEntry {
    bool finalized_;
    int size_;
};

struct LogCommit {
    size_t total_size_;
    size_t log_size_;
    int num_blocks_;
    Block next_;
    Block blocks_[];

    static int GetSize(int num_blocks, size_t log_size) {
        return sizeof(LogCommit) + num_blocks*sizeof(Block) + log_size;
    }

    char* GetLogOff() {
        return reinterpret_cast<char*>(LABSTOR_REGION_ADD(sizeof(LogCommit) + num_blocks_*sizeof(Block), this));
    }
};

struct CoreLog {
    LogEntry *head_, *tail_, *reserve_off_, *commit_off_;
    BlockAllocator alloc_;
    uint64_t file_uid_min_;

    CoreLog(size_t log_size, size_t disk_off, size_t disk_size, size_t num_small_blocks, void *region, size_t region_size) {
        //Log entries
        head_ = reinterpret_cast<LogEntry*>(region);
        tail_ = reinterpret_cast<LogEntry*>(LABSTOR_REGION_ADD(region_size, region));
        reserve_off_ = head_;
        commit_off_ = head_;

        //Block allocator
        alloc_.Initialize(disk_off, disk_size, num_small_blocks, tail_, region_size - log_size);
    }

    template<typename T>
    T* ReserveLogEntry() {
        T* entry = reinterpret_cast<T*>(reserve_off_ + 1);
        entry->size_ = sizeof(T);
        entry->finalized_ = false;
        reserve_off_ = reinterpret_cast<LogEntry*>(entry + 1);
        return entry;
    }

    template<typename T>
    void FinalizeLogEntry(T *entry) {
        entry->finalize_ = true;
    }

    void GetBlock(int size, Block &block) {
        alloc_.GetBlock(size, block);
    }

    void FreeBlock(Block &block) {
        alloc_.FreeBlock(block);
    }

    size_t GetUncommittedSize() {
        return (size_t)reserve_off_ - (size_t)commit_off_;
    }

    LogEntry* GetUncommittedOff() {
        return commit_off_;
    }
};

struct Inode {
    std::list<Block> blocks_;
};

class Log {
    int region_id_;
    void *region_;
    std::vector<CoreLog> per_core_log_;
    std::unordered_map<std::string, uint64_t> path_to_id_;
    Block next_log_block_;
public:
    Log() = default;

    void Initialize(size_t log_size, size_t disk_size, size_t num_small_blocks, int concurrency) {
        size_t disk_off = SMALL_BLOCK_SIZE;
        size_t per_core_log_size = log_size / concurrency;
        size_t per_core_disk_size = (disk_size - SMALL_BLOCK_SIZE)/concurrency;
        size_t per_core_num_blocks = num_small_blocks/concurrency;
        size_t per_core_region_size = per_core_log_size + BlockAllocator::GetSize(per_core_disk_size, per_core_num_blocks);
        size_t region_size = per_core_region_size * concurrency;

        //Shared-memory Region
        /*LABSTOR_KERNEL_SHMEM_ALLOC_T shmem = LABSTOR_KERNEL_SHMEM_ALLOC;
        region_id_ = shmem->CreateShmem(region_size, true);
        shmem->GrantPidShmem(getpid(), region_id_);
        region_ = shmem->MapShmem(region_id_, region_size);
        void *section = region_;*/
        void *region = malloc(region_size);

        //LabFS Operation Log & Block Allocator
        per_core_log_.reserve(concurrency);
        for(int i = 0; i < concurrency; ++i) {
            per_core_log_.emplace_back(per_core_log_size, disk_off, per_core_disk_size, per_core_num_blocks, region, per_core_region_size);
            disk_off += per_core_disk_size;
        }

        //LabFS Path to Inode mapping
    }

    void SetFirstLogBlock(Block &&block) {
        next_log_block_ = block;
    }

    CoreLog& GetCoreLog() {
        return per_core_log_[labstor::ThreadLocal::GetTid()];
    }

    void GetLogUpdates(LogCommit *&update, size_t size, const Block &first_block) {
        //Get all uncommitted changes to the log
        size_t log_size = 0;
        for(auto &core_log : per_core_log_) {
            log_size += core_log.GetUncommittedSize();
        }

        //Get the first block
        int disk_size = log_size;
        std::list<Block> blocks;
        blocks.emplace_back(next_log_block_);
        disk_size -= next_log_block_.size_;

        //Allocate blocks for storing the current log
        auto &core_log = GetCoreLog();
        while(disk_size > 0) {
            Block block;
            if (disk_size > LARGE_BLOCK_SIZE) {
                core_log.GetBlock(LARGE_BLOCK_SIZE, block);
                disk_size -= LARGE_BLOCK_SIZE;
            } else {
                core_log.GetBlock(SMALL_BLOCK_SIZE, block);
                disk_size -= SMALL_BLOCK_SIZE;
            }
            blocks.emplace_back(block);
        }

        //Allocate next block
        core_log.GetBlock(SMALL_BLOCK_SIZE, next_log_block_);

        //Create the LogCommit message
        update = reinterpret_cast<LogCommit*>(malloc(LogCommit::GetSize(blocks.size(), log_size)));
        update->num_blocks_ = 0;
        update->log_size_ = 0;
        update->next_ = next_log_block_;
        for(auto &block : blocks) {
            update->blocks_[update->num_blocks_++];
        }
        char *log_off = update->GetLogOff();
        for(auto &core_log : per_core_log_) {
            size_t uncommitted_size = core_log.GetUncommittedSize();
            memcpy(log_off, core_log.GetUncommittedOff(), uncommitted_size);
            update->log_size_ += uncommitted_size;
        }
    }
};

}

#endif //LABSTOR_LABSTOR_FS_LOG_H
