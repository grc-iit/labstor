//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/util/timer.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>

#include <unistd.h>
#include <fcntl.h>
#include <aio.h>

void test_throughput(int total_reqs) {
    labstor::ipc::queue_pair q;
    labstor::ipc::request *rq;
    labstor::ipc::request *req_region;
    size_t sq_size, cq_size;
    void *region, *sq_region, *cq_region;
    labstor::ipc::qtok_t qtok;
    labstor::Timer t[4];
    int queue_depth = total_reqs;

    //Allocate region & initialize queue
    LABSTOR_ERROR_HANDLE_START()
    sq_size = labstor::ipc::request_queue::GetSize(queue_depth);
    cq_size = labstor::ipc::request_map::GetSize(queue_depth);
    region = malloc(sq_size + cq_size + total_reqs*sizeof(labstor::ipc::request));
    sq_region = region;
    cq_region = LABSTOR_REGION_ADD(sq_size, region);
    req_region = (labstor::ipc::request*)LABSTOR_REGION_ADD(cq_size, cq_region);
    q.Init(0, region, queue_depth, sq_region, sq_size, cq_region, cq_size);
    LABSTOR_ERROR_HANDLE_END()

    printf("Starting\n");

    //Enqueue
    for(int i = 0; i < total_reqs; ++i) {
        t[0].Resume();
        q.Enqueue(&req_region[i], qtok);
        t[0].Pause();
    }

    //Dequeue
    for(int i = 0; i < total_reqs; ++i) {
        t[1].Resume();
        q.Dequeue(rq);
        t[1].Pause();
    }


    printf("total_reqs=%d, enqueue=%lf,dequeue=%lf",
           total_reqs,
           total_reqs/t[0].GetMsec(),
           total_reqs/t[1].GetMsec()
   );
}

int main(int argc, char **argv) {
    test_throughput(100000);
    return 0;
}