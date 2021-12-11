//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/util/timer.h>
#include <modules/kernel/ipc_test/client/ipc_test_client.h>

#include <unistd.h>

void PrintBitmap(labstor_bitmap_t *bitmap, int num_bits) {
    for(int i = 0; i < num_bits; ++i) {
        if(i%64 == 0) {
            printf("\n");
        }
        printf("%d", labstor_bitmap_IsSet(bitmap, i));
    }
}

void produce_and_consume(bool consume) {
    labstor::ipc::request_queue q;
    labstor::ipc::request *rq;
    labstor::ipc::request *req_region;
    std::vector<int> being_set, was_set, was_dequeued;
    int nthreads, last_thread, reqs_per_thread, total_reqs, total_queued_reqs;
    size_t queue_size;
    void *region;

    LABSTOR_ERROR_HANDLE_START()
    nthreads = 64;
    last_thread = nthreads - 1;
    reqs_per_thread = 10000;
    total_reqs = reqs_per_thread * (nthreads - 1);
    total_queued_reqs = 256;
    queue_size = labstor::ipc::request_queue::GetSize(total_queued_reqs);
    region = malloc(queue_size + total_reqs*sizeof(labstor::ipc::request));
    req_region = (labstor::ipc::request*)((char*)region + queue_size);
    LABSTOR_ERROR_HANDLE_END()

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

    LABSTOR_ERROR_HANDLE_START()
    q.Init(region, region, queue_size, total_queued_reqs, 10);
    LABSTOR_ERROR_HANDLE_END()

    printf("Starting\n");

    //Spam the trusted server
    omp_set_dynamic(0);
#pragma omp parallel shared(being_set, was_set, was_dequeued, total_reqs, reqs_per_thread, nthreads, q) num_threads(nthreads)
    {
        LABSTOR_ERROR_HANDLE_START()
            int rank = omp_get_thread_num();

            if(rank != last_thread) {
                for (int i = 0; i < reqs_per_thread; ++i) {
                    int off = rank*reqs_per_thread + i;
                    req_region[off].ns_id_ = off;
                    being_set[off] = 1;
                    //printf("EnqueueStart[X] = %d\n", off);
                    labstor::ipc::qtok_t qtok = q.Enqueue(req_region + off);
                    was_set[off] = 1;
                    if(qtok.req_id == -1) {
                        printf("Failed to queue request %d\n", off);
                        exit(1);
                    }
                }
            } else if(rank == last_thread && consume) {
                labstor::Timer t;
                t.Resume();
                int i = 0;
                while (i < total_reqs && t.GetMsecFromStart() < 500) {
                    if(q.GetDepth()) {
                        //printf("Depth: %d Enqueued: %d Dequeued: %d\n", q.GetDepth(), q.queue_.header_->enqueued_, q.queue_.header_->dequeued_);
                        t.Resume();
                    }
                    if (!q.Dequeue(rq)) { continue; }
                    ++was_dequeued[rq->ns_id_];
                    ++i;
                }
                printf("No longer digesting!\n");

                for(int i = 0; i < was_set.size(); ++i) {
                    if(being_set[i] && !was_set[i]) {
                        printf("Entry[%d] is hanging!\n", i);
                    }
                }

                printf("Queue Depth: %d\n", q.GetDepth());
                PrintBitmap(q.queue_.header_->bitmap_, total_queued_reqs);

                if(i < total_reqs) {
                    exit(1);
                }
            }
            printf("Done[%d]\n", rank);
#pragma omp barrier
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
    produce_and_consume(true);
    return 0;
}