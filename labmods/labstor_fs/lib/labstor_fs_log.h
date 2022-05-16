//
// Created by lukemartinlogan on 3/13/22.
//

//We have to write 256KB of log data to disk. How to do while detecting faults?
    //Can we assume that some size of I/O is guaranteed to complete when issued?
    //If not, there is no real way?

#ifndef LABSTOR_LABSTOR_FS_LOG_H
#define LABSTOR_LABSTOR_FS_LOG_H

/*
 * We have a device with 1TB of data; how do we divide blocks on the storage device?
 * First 64MB is a log, per-say
 * */

#include <vector>
#include <list>
#include <labmods/secure_shmem/netlink_client/secure_shmem_client_netlink.h>
#include <labstor/types/allocator/allocator.h>
#include <labstor/types/allocator/shmem_allocator.h>
#include "block_allocator.h"
#include "labstor/types/data_structures/unordered_map/shmem_inode_map.h"
#include <labstor/types/allocator/shmem_allocator.h>

namespace labstor::LabFS {

struct Inode {
    uint64_t uuid_;
    std::list<Block> blocks_;
};

struct LogEntry {
    bool finalized_;
    int size_;
};

struct LogCommit {
    uint64_t checksum_;
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
    uint64_t uuid_min_;

    CoreLog(size_t uuid_min, size_t log_size, size_t disk_off, size_t disk_size, size_t num_small_blocks, void *region, size_t region_size) {
        //Log entries
        head_ = reinterpret_cast<LogEntry*>(region);
        tail_ = reinterpret_cast<LogEntry*>(LABSTOR_REGION_ADD(region_size, region));
        reserve_off_ = head_;
        commit_off_ = head_;
        uuid_min_ = uuid_min;

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

    uint64_t GetUUID() {
        return uuid_min_++;
    }

    size_t GetUncommittedSize() {
        return (size_t)reserve_off_ - (size_t)commit_off_;
    }

    LogEntry* GetUncommittedOff() {
        return commit_off_;
    }
};

/*
 * When searching for an Inode, check path
 * Directories store a list of file UUIDs they contain in-memory
 * */

class Log {
    int region_id_;
    void *region_;
    std::vector<CoreLog> per_core_log_;
    labstor::GenericAllocator *shmem_alloc_;
    labstor::ipc::mpmc::inode_map path_to_inode_;
    std::unordered_map<std::pair<int,int>, uint32_t> fd_to_inode_;
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
        size_t uuid_diff = (uint64_t)(-1)/concurrency, cur_uuid = 1; //Root UUID is 0

        //Shared-memory Region
        /*LABSTOR_KERNEL_SHMEM_ALLOC_T shmem = LABSTOR_KERNEL_SHMEM_ALLOC;
        region_id_ = shmem->CreateShmem(region_size, true);
        shmem->GrantPidShmem(getpid(), region_id_);
        region_ = shmem->MapShmem(region_id_, region_size);
        void *section = region_;*/
        void *region = malloc(region_size);

        //Set the first log block
        SetLogBlock(Block(0,SMALL_BLOCK_SIZE));

        //LabFS Operation Log & Block Allocator
        per_core_log_.reserve(concurrency);
        for(int i = 0; i < concurrency; ++i) {
            per_core_log_.emplace_back(cur_uuid, per_core_log_size, disk_off, per_core_disk_size, per_core_num_blocks, region, per_core_region_size);
            disk_off += per_core_disk_size;
            cur_uuid += uuid_diff;
        }

        //LabFS Path to Inode mapping
    }

    void Attach(void *region) {
    }

    void SetLogBlock(Block &&block) {
        next_log_block_ = block;
    }

    Inode* FindInode(uint64_t dir_uid, labstor::ipc::string filename)  {
        uint32_t inode_off_;
        if(path_to_inode_.Find(std::pair(dir_uid, filename), inode_off_)) {
            return nullptr;
        }
    }

    Inode FindInode(int pid, int fd)  {
        return fd_to_inode_[std::pair(pid,fd)];
    }

    void RemoveInode(int pid, int fd) {
        Inode *inode = FindInode(pid,fd);
        fd_to_inode_.erase(std::pair(pid,fd));
    }

    void CreateInode(uint64_t dir_uid, labstor::ipc::string path) {
    }

    Block& GetLogBlock() {
        return next_log_block_;
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
