//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/util/timer.h>
#include <modules/kernel/ipc_test/client/ipc_test_client.h>

#include <unistd.h>

int num_valid_entries(labstor::ipc::request_map &map) {
    int count = 0;
    for(int i = 0; i < map.GetNumBuckets(); ++i) {
        if(labstor_bit2map_IsSet(map.bitmap_, i, UNORDERED_MAP_VALID)) {
            ++count;
        }
    }
    return count;
}

void produce_and_consume(bool consume) {
    size_t half_region_size = 1<<20;
    labstor::ipc::request_map map;
    void *region = malloc(2*half_region_size);
    labstor::ipc::request *req_region = (labstor::ipc::request*)((char*)region + half_region_size);
    int nthreads = 16;
    int last_thread = nthreads - 1;
    int reqs_per_thread = 4096;
    int total_reqs = reqs_per_thread * (nthreads - 1);
    std::vector<int> was_set;
    std::vector<int> responses;
    was_set.resize(total_reqs);
    responses.resize(total_reqs);
    for(int i = 0; i < total_reqs; ++i) {
        responses[i] = 0;
        was_set[i] = 0;
    }
    if(!consume) {
        map.Init(region, region, labstor::ipc::request_map::GetSize(total_reqs), total_reqs);
    } else {
        map.Init(region, region, labstor::ipc::request_map::GetSize(total_reqs - 1024), 16);
    }

    printf("Starting\n");

    //Spam the trusted server
    omp_set_dynamic(0);
#pragma omp parallel shared(responses, req_region, total_reqs, reqs_per_thread, nthreads, map) num_threads(nthreads)
    {
        LABSTOR_ERROR_HANDLE_START()
            int rank = omp_get_thread_num();

            if(rank != last_thread) {
                labstor::Timer t;
                t.Resume();
                for (int i = 0; i < reqs_per_thread; ++i) {
                    labstor::ipc::request *rq;
                    int off = rank*reqs_per_thread + i;
                    rq = req_region + off;
                    rq->req_id_ = off;
                    //printf("BeginSet[rid=%d,off=%d,i=%d,rank=%d,req_region=%p,rq=%p]\n",rq->req_id_,off,i, rank, req_region, rq);
                    while(!map.Set(rq) && t.GetMsecFromStart() < 1000);
                    if(t.GetMsecFromStart() >= 1000) {
                        printf("Could not set request, map was full\n");
                        exit(1);
                    }
                    //printf("WasSet[%d,%d]\n",off,rq->req_id_);
                    ++was_set[rq->req_id_];
                }
            }
            else if(rank == last_thread && !consume) {
                labstor::Timer t;
                t.Resume();
                labstor::ipc::request *rq;
                int i;
                do {
                    i = 0;
                    for(int j = 0; j < total_reqs; ++j) {
                        if (!map.Find(j,rq)) { break; }
                        ++i;
                    }
                } while (i != total_reqs && t.GetMsecFromStart() < 5000);
            }
            else if(rank == last_thread && consume) {
                labstor::Timer t;
                t.Resume();
                labstor::ipc::request *rq;
                int i = 0;
                while (i != total_reqs && t.GetMsecFromStart() < 500) {
                    if(num_valid_entries(map)) {
                        t.Resume();
                    }
                    for(int j = 0; j < total_reqs; ++j) {
                        if (!map.Find(j,rq)) { continue; }
                        if (!map.Remove(j)) { continue; }
                        responses[rq->req_id_] = 1;
                        ++i;
                    }
                    i=0;
                }
                printf("Done?\n");
            }
#pragma omp barrier
        LABSTOR_ERROR_HANDLE_END()
    }

    bool fail = false;
    for (int i = 0; i < responses.size(); ++i) {
        if (was_set[i] == 0) {
            printf("Error, response %d was never set\n", i);
            fail = true;
        }
    }
    if(fail) exit(1);
    printf("All map entries were set\n");

    if(!consume) {
        labstor::ipc::request *rq;
        for(int j = 0; j < total_reqs; ++j) {
            if (!map.Find(j, rq)) {
                printf("Could not find element %d\n", j);
                exit(1);
            }
        }
        printf("Success!\n");
    }
    if(consume) {
        int num_null = 0;
        for (int i = 0; i < responses.size(); ++i) {
            if (responses[i] == 0) {
                printf("Error, response %d was null\n", i);
                fail = true;
                ++num_null;
            } else if (responses[i] > 1) {
                printf("Error, response %d was received multiple times\n", i);
                fail = true;
            }
        }
        printf("%d/%d were not set\n", num_null, responses.size());
        if(fail) exit(1);
        printf("Success\n");
    }
}

int main(int argc, char **argv) {
    produce_and_consume(true);
    return 0;
}