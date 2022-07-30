
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

#include <labstor/userspace/server/ipc_manager.h>

int main() {
    /*labstor::ipc::mpmc::int_map<labstor::ipc::qid_t,labstor::queue_pair*> qps_by_id_;
    uint32_t region_size = (1<<20);
    int queue_size = 2048;
    void *region = malloc(region_size);
    void *sq_region, *cq_region;
    int num_queues = 64;

    printf("QP SIZE: %lu\n", sizeof(labstor::queue_pair));

    qps_by_id_.Init(region, region, region_size, 16);
    labstor::queue_pair *qp;
    for(int i = 0; i < num_queues; ++i) {
        //Initialize QP
        labstor::ipc::qid_t qid = labstor::queue_pair::GetQID(
                LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                i,
                num_queues,
                KERNEL_PID);
        void *region = malloc(2*queue_size);
        void *sq_region = region;
        void *cq_region = (char*)region + queue_size;
        qp->Init(qid, region, sq_region, queue_size, cq_region, queue_size);
        printf("%d\n", qp->cq_.GetNumBuckets());

        //Store QP internally
        qps_by_id_.Set(qid, qp);
    }

    labstor::queue_pair *kern_qp;
    labstor::ipc::qid_t qid = labstor::queue_pair::GetQID(
            LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
            4,
            num_queues,
            KERNEL_PID);
    kern_qp = qps_by_id_[qid];
    printf("%d\n", kern_qp->cq_.GetNumBuckets());*/
}