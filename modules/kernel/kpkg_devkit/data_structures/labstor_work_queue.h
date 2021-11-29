//
// Created by lukemartinlogan on 11/29/21.
//

#ifndef LABSTOR_WORK_QUEUE_H
#define LABSTOR_WORK_QUEUE_H

class work_queue : public shmem_type {
private:
    ring_buffer<queue_pair_ptr> queue_;
    LABSTOR_IPC_MANAGER_T ipc_manager_;
public:
    inline work_queue() {
        ipc_manager_ = LABSTOR_IPC_MANAGER;
    }

    inline void* GetRegion() override { return queue_.GetRegion(); }
    static uint32_t GetSize(uint32_t num_buckets) {
        return ring_buffer<queue_pair_ptr>::GetSize(num_buckets);
    }
    inline uint32_t GetSize() override {
        return queue_.GetSize();
    }

    inline void Init(void *region, uint32_t region_size) {
        queue_.Init(region, region_size);
    }

    inline void Attach(void *region) {
        queue_.Attach(region);
    }

    inline bool Enqueue(queue_pair &qp, void *base) {
        queue_pair_ptr ptr;
        qp.GetPointer(ptr, base);
        return queue_.Enqueue(ptr);
    }

    inline bool Dequeue(queue_pair &qp, void *&base, labstor::credentials *&creds) {
        queue_pair_ptr ptr;
        if(!queue_.Dequeue(ptr)) {
            return false;
        }
        base = ipc_manager_->GetRegion(ptr, creds);
        qp.Attach(ptr, base);
        return true;
    }

    inline uint32_t GetLength() {
        return queue_.GetDepth();
    }
};

#endif //LABSTOR_WORK_QUEUE_H
