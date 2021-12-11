//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/util/timer.h>
#include <modules/kernel/ipc_test/client/ipc_test_client.h>

#include <unistd.h>

void produce_and_consume(bool consume) {
    labstor::ipc::request_queue q;
    labstor::ipc::request *rq;
    labstor::ipc::request *req_region;
    std::vector<int> responses;
    int nthreads, last_thread, reqs_per_thread, total_reqs, total_queued_reqs;
    size_t queue_size;
    void *region;

    LABSTOR_ERROR_HANDLE_START()
    nthreads = 64;
    last_thread = nthreads - 1;
    reqs_per_thread = 10000;
    total_reqs = reqs_per_thread * (nthreads - 1);
    total_queued_reqs = 4096;
    queue_size = labstor::ipc::request_queue::GetSize(total_queued_reqs);
    region = malloc(queue_size + total_reqs*sizeof(labstor::ipc::request));
    req_region = (labstor::ipc::request*)((char*)region + queue_size);
    LABSTOR_ERROR_HANDLE_END()

    LABSTOR_ERROR_HANDLE_START()
    responses.resize(total_reqs);
    for(int i = 0; i < total_reqs; ++i) {
        responses[i] = 0;
    }
    LABSTOR_ERROR_HANDLE_END()

    LABSTOR_ERROR_HANDLE_START()
    q.Init(region, region, queue_size, total_queued_reqs, 10);
    LABSTOR_ERROR_HANDLE_END()

    printf("Starting\n");

    //Spam the trusted server
    omp_set_dynamic(0);
#pragma omp parallel shared(responses, total_reqs, reqs_per_thread, nthreads, q) num_threads(nthreads)
    {
        LABSTOR_ERROR_HANDLE_START()
            int rank = omp_get_thread_num();

            if(rank != last_thread) {
                labstor::Timer t;
                for (int i = 0; i < reqs_per_thread; ++i) {
                    int off = rank*reqs_per_thread + i;
                    req_region[off].ns_id_ = off;
                    labstor::ipc::qtok_t qtok = q.Enqueue(req_region + off);
                    if(qtok.req_id == -1) {
                        printf("Failed to queue request %d\n", off);
                        exit(1);
                    }
                }
            } else if(rank == last_thread && consume) {
                labstor::Timer t;
                t.Resume();
                while (t.GetMsecFromStart() < 5000) {
                    if (!q.Dequeue(rq)) { continue; }
                    ++responses[rq->ns_id_];
                }
            }
            printf("Done[%d]\n", rank);
#pragma omp barrier
        LABSTOR_ERROR_HANDLE_END()
    }

    if(consume) {
        for (int i = 0; i < responses.size(); ++i) {
            if (responses[i] == 0) {
                printf("Error, response %d was null\n", i);
                exit(1);
            } else if (responses[i] > 1) {
                printf("Error, response %d was received multiple times\n", i);
                exit(1);
            }
        }
        printf("Success\n");
    } else {
        printf("QP DEPTH: %d vs %d\n", q.GetDepth(), total_reqs);
        if(total_reqs != q.GetDepth()) {
            printf("Requests were dropped\n");
            exit(1);
        } else {
            printf("Success\n");
        }
    }
}

int main(int argc, char **argv) {
    produce_and_consume(true);
    return 0;
}