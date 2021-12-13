//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/util/timer.h>
#include <labstor/types/data_structures/mpmc/shmem_queue_pair.h>

#include <unistd.h>

int num_valid_entries(labstor::ipc::request_map &map) {
    int count = 0;
    for(int i = 0; i < map.GetNumBuckets(); ++i) {
        if(labstor_bit2map_IsSet(map.bitmap_, i, LABSTOR_BIT2MAP_VALID)) {
            ++count;
        }
    }
    return count;
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

void produce_and_consume(int num_producers, int num_consumers, int total_reqs, int num_buckets, int max_collisions) {
    labstor::ipc::request_map map;
    int reqs_per_thread, map_size, request_alloc_size;
    int nthreads;

    //Initial numbers
    total_reqs = (total_reqs/num_producers)*num_producers;
    reqs_per_thread = total_reqs / num_producers;
    nthreads = num_producers + num_consumers;

    //Allocate map & requests
    map_size = labstor::ipc::request_map::GetSize(total_reqs);
    request_alloc_size = total_reqs * sizeof(labstor::ipc::request);
    void *region = malloc(map_size + request_alloc_size);
    labstor::ipc::request *req_region = (labstor::ipc::request*)((char*)region + map_size);
    map.Init(region, region, labstor::ipc::request_map::GetSize(num_buckets), max_collisions);

    //Initialize trackers
    std::vector<int> being_set;
    std::vector<int> was_set;
    std::vector<int> was_dequeued;
    being_set.resize(total_reqs);
    was_set.resize(total_reqs);
    was_dequeued.resize(total_reqs);
    for(int i = 0; i < total_reqs; ++i) {
        was_dequeued[i] = 0;
        being_set[i] = 0;
        was_set[i] = 0;
    }

    printf("Starting\n");

    //Spam the trusted server
    omp_set_dynamic(0);
#pragma omp parallel shared(was_dequeued, req_region, total_reqs, reqs_per_thread, nthreads, map) num_threads(nthreads)
    {
        LABSTOR_ERROR_HANDLE_START()
            int rank = omp_get_thread_num();

            if(rank < num_producers) {
                labstor::Timer t;
                t.Resume();
                for (int i = 0; i < reqs_per_thread; ++i) {
                    labstor::ipc::request *rq;
                    int off = rank*reqs_per_thread + i;
                    rq = req_region + off;
                    rq->req_id_ = off;
                    ++being_set[rq->req_id_];
                    printf("Enqueuing[%d]\n", rq->req_id_);
                    while(!map.Set(rq));
                    printf("FinishedEnqueuing[%d]\n", rq->req_id_);
                    ++was_set[rq->req_id_];
                }
            }
            else if(rank > num_producers) {
                labstor::Timer t;
                t.Resume();
                labstor::ipc::request *rq;
                int i = 0;
                //while (1) {
                //while (i < total_reqs && t.GetMsecFromStart() < 10000) {
                while (i < total_reqs && t.GetMsecFromStart() < 10000) {
                    if (num_valid_entries(map)) {
                        t.Resume();
                    }
                    for (int j = 0; j < total_reqs; ++j) {
                        if (!map.Find(j, rq)) { continue; }
                        if (!map.Remove(j)) { continue; }
                        printf("Dequeuing[%d]\n", rq->req_id_);
                        ++was_dequeued[rq->req_id_];
                    }
                    i = GetNumCompleted(was_dequeued);
                }
                printf("StopIngesting[%d]\n", rank);
            }
#pragma omp barrier
        LABSTOR_ERROR_HANDLE_END()
    }

    int num_unset = 0;
    for (int i = 0; i < was_dequeued.size(); ++i) {
        if (was_set[i] == 0) {
            ++num_unset;
        }
    }
    if(num_unset) {
        printf("%d/%d entries didn't complete being set\n", num_unset, total_reqs);
        printf("%d/%d entries were set\n", total_reqs - num_unset, total_reqs);
        exit(1);
    }
    printf("All map entries were set\n");

    int num_null = 0;
    bool fail = false;
    for (int i = 0; i < was_dequeued.size(); ++i) {
        if (was_dequeued[i] == 0) {
            printf("Request %d was never dequeued\n", i);
            ++num_null;
            fail = true;
        } else if (was_dequeued[i] > 1) {
            printf("Request %d was received multiple times\n", i);
            fail = true;
        }
    }
    printf("%d/%lu were not set\n", num_null, was_dequeued.size());
    if(fail) exit(1);
    printf("Success\n");
}

int main(int argc, char **argv) {
    produce_and_consume(32, 32, 1<<15, 256, 16);
    return 0;
}