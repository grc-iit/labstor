//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/util/timer.h>
#include "labstor/types/data_structures/c/shmem_queue_pair.h"

#include <unistd.h>

void PrintBitmap(labstor_bitmap_t *bitmap, int num_bits) {
    for(int i = 0; i < num_bits; ++i) {
        if(i%64 == 0) {
            printf("\n");
        }
        printf("%d", labstor_bitmap_IsSet(bitmap, i));
    }
}

int GetNumCompleted(std::vector<int> &was_dequeued) {
    int count = 0;
    for(int i = 0; i < was_dequeued.size(); ++i) {
        if(was_dequeued[i]) {
            ++count;
        }
    }
    return count;
}

void produce_and_consume(bool consume, int total_reqs, int num_producers, int num_consumers, int queue_depth) {
    labstor::queue_pair q;
    labstor::ipc::request *rq;
    labstor::ipc::request *req_region;
    std::vector<int> being_queued, was_queued, was_dequeued, being_completed, was_completed, was_acked;
    int reqs_per_producer, nthreads = num_producers + num_consumers;
    size_t sq_size, cq_size;
    void *region, *sq_region, *cq_region;

    //Get total number of requests and reqs per thread
    total_reqs = num_producers * (total_reqs/num_producers);
    reqs_per_producer = total_reqs/num_producers;
    printf("%d %d\n", reqs_per_producer, total_reqs);

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

    //Initialize trackers
    LABSTOR_ERROR_HANDLE_START()
    being_queued.resize(total_reqs);
    was_queued.resize(total_reqs);
    was_dequeued.resize(total_reqs);
    being_completed.resize(total_reqs);
    was_completed.resize(total_reqs);
    was_acked.resize(total_reqs);
    for(int i = 0; i < total_reqs; ++i) {
        being_queued[i] = 0;
        was_queued[i] = 0;
        was_dequeued[i] = 0;
        being_completed[i] = 0;
        was_completed[i] = 0;
        was_acked[i] = 0;
    }
    LABSTOR_ERROR_HANDLE_END()

    printf("Starting\n");

    //Spam the trusted server
    omp_set_dynamic(0);
#pragma omp parallel shared(being_queued, was_queued, was_dequeued, total_reqs, reqs_per_producer, nthreads, q) num_threads(nthreads)
    {
        int i = 0;
        LABSTOR_ERROR_HANDLE_START()
            int rank = omp_get_thread_num();
            if(rank < num_producers) {
                for (int i = 0; i < reqs_per_producer; ++i) {
                    int off = rank*reqs_per_producer + i;
                    req_region[off].ns_id_ = off;
                    being_queued[off] = 1;
                    printf("ToEnqueue: %d\n", off);
                    labstor::ipc::qtok_t qtok;
                    q.Enqueue(req_region + off, qtok);
                    printf("DidEnqueue: %d\n", off);
                    q.Wait<labstor::ipc::request>(qtok);
                }
                printf("EnqueuingDone[%d]\n", rank);
            } else if(rank >= num_producers && consume) {
                labstor::HighResMonotonicTimer t;
                t.Resume();
                while (i < total_reqs) {
                //while (i < total_reqs && t.GetMsecFromStart() < 5000) {
                    for(int j = 0; j < 1000; ++j) {
                        if (q.GetDepth()) {
                            printf("Depth: %d\n", q.GetDepth());
                            t.Resume();
                        }
                        if (!q.Dequeue(rq)) { continue; }
                        q.Complete(rq, rq);
                        ++was_dequeued[rq->ns_id_];
                    }
                    i = GetNumCompleted(was_dequeued);
                }
                printf("%d longer digesting!\n", rank);
            }

#pragma omp barrier
            if(rank == num_producers) {
                for(int i = 0; i < was_queued.size(); ++i) {
                    if(being_queued[i] && !was_queued[i]) {
                        printf("Entry[%d] is hanging!\n", i);
                    }
                }

                printf("Queue Depth: %d\n", q.GetDepth());

                i = GetNumCompleted(was_dequeued);
                if(i < total_reqs) {
                    printf("Only %d/%d requests completed?\n", i, total_reqs);
                    exit(1);
                }
            }
        LABSTOR_ERROR_HANDLE_END()
    }

    if(consume) {
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
    produce_and_consume(true, (1<<18), 62, 4, 256);
    return 0;
}