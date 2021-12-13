//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/util/timer.h>
#include <modules/kernel/ipc_test/client/ipc_test_client.h>

#include <unistd.h>

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
    labstor::ipc::request_queue q;
    labstor::ipc::request *rq;
    labstor::ipc::request *req_region;
    std::vector<int> being_set, was_set, was_dequeued;
    int reqs_per_producer, nthreads = num_producers + num_consumers;
    size_t queue_size;
    void *region;

    //Get total number of requests and reqs per thread
    total_reqs = num_producers * (total_reqs/num_producers);
    reqs_per_producer = total_reqs/num_producers;
    printf("%d %d\n", reqs_per_producer, total_reqs);

    //Allocate region
    LABSTOR_ERROR_HANDLE_START()
    queue_size = labstor::ipc::request_queue::GetSize(queue_depth);
    region = malloc(queue_size + total_reqs*sizeof(labstor::ipc::request));
    req_region = (labstor::ipc::request*)((char*)region + queue_size);
    LABSTOR_ERROR_HANDLE_END()

    //Initialize trackers
    LABSTOR_ERROR_HANDLE_START()
    being_set.resize(total_reqs);
    was_set.resize(total_reqs);
    was_dequeued.resize(total_reqs);
    for(int i = 0; i < total_reqs; ++i) {
        being_set[i] = 0;
        was_set[i] = 0;
        was_dequeued[i] = 0;
    }
    LABSTOR_ERROR_HANDLE_END()

    //Initialize queue
    LABSTOR_ERROR_HANDLE_START()
    q.Init(region, region, queue_size, queue_depth, 10);
    LABSTOR_ERROR_HANDLE_END()

    printf("Starting\n");

    //Spam the trusted server
    omp_set_dynamic(0);
#pragma omp parallel shared(being_set, was_set, was_dequeued, total_reqs, reqs_per_producer, nthreads, q) num_threads(nthreads)
    {
        int i = 0;

        LABSTOR_ERROR_HANDLE_START()
            int rank = omp_get_thread_num();
            if(rank < num_producers) {
                for (int i = 0; i < reqs_per_producer; ++i) {
                    int off = rank*reqs_per_producer + i;
                    req_region[off].ns_id_ = off;
                    being_set[off] = 1;
                    //printf("EnqueueStart[X] = %d\n", off);
                    labstor::ipc::qtok_t qtok;
                    if(q.Enqueue(req_region + off, qtok)) {
                        was_set[off] = 1;
                    } else {
                        printf("Failed to queue request %d\n", off);
                        exit(1);
                    }
                }
                printf("EnqueuingDone[%d]\n", rank);
            } else if(rank >= num_producers && consume) {
                labstor::Timer t;
                t.Resume();
                while (i < total_reqs) {
                //while (i < total_reqs && t.GetMsecFromStart() < 5000) {
                    for(int j = 0; j < 1000; ++j) {
                        if (q.GetDepth()) {
                            //printf("Depth: %d Enqueued: %d Dequeued: %d\n", q.GetDepth(), q.queue_.header_->enqueued_, q.queue_.header_->dequeued_);
                            t.Resume();
                        }
                        if (!q.Dequeue(rq)) { continue; }
                        ++was_dequeued[rq->ns_id_];
                    }
                    i = GetNumCompleted(was_dequeued);
                }
                printf("%d longer digesting!\n", rank);
            }

#pragma omp barrier
            if(rank == num_producers) {
                for(int i = 0; i < was_set.size(); ++i) {
                    if(being_set[i] && !was_set[i]) {
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
    produce_and_consume(true, 4096, 1, 1, 256);
    return 0;
}