//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/userspace/server/ipc_manager.h>

int main() {
    labstor::ipc::mpmc::int_map_labstor_qid_t_qp qps_by_id_;
    uint32_t region_size = (1<<20);
    int queue_size = 2048;
    void *region = malloc(region_size);
    void *sq_region, *cq_region;
    int num_queues = 64;

    printf("QP SIZE: %lu\n", sizeof(labstor::ipc::queue_pair));

    qps_by_id_.Init(region, region, region_size, 16);
    labstor::ipc::queue_pair *qp;
    for(int i = 0; i < num_queues; ++i) {
        //Initialize QP
        labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(
                LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                i,
                num_queues,
                KERNEL_PID);
        void *region = malloc(2*queue_size);
        void *sq_region = region;
        void *cq_region = (char*)region + queue_size;
        qp->Init(qid, region, sq_region, queue_size, cq_region, queue_size);
        printf("%d\n", qp->cq.GetNumBuckets());

        //Store QP internally
        qps_by_id_.Set(qid, qp);
    }

    labstor::ipc::queue_pair *kern_qp;
    labstor::ipc::qid_t qid = labstor::ipc::queue_pair::GetStreamQueuePairID(
            LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
            4,
            num_queues,
            KERNEL_PID);
    kern_qp = qps_by_id_[qid];
    printf("%d\n", kern_qp->cq.GetNumBuckets());
}