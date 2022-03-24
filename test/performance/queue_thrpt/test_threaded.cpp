//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/util/timer.h>
#include "labstor/types/data_structures/c/shmem_queue_pair.h"
#include <labstor/types/thread_local.h>
#include <labstor/userspace/util/partitioner.h>

#include <fcntl.h>
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

void produce_and_consume(int total_reqs, int num_producers, int num_consumers, int dedicate, int queue_depth) {
    labstor::HighResMonotonicTimer t;
    labstor::ipc::shmem_queue_pair qps[num_producers];
    labstor::ipc::request *rq;
    labstor::ipc::request *req_region;
    int reqs_per_producer, nthreads = num_producers + num_consumers;
    size_t sq_size, cq_size;
    size_t sq_region_size, cq_region_size, req_region_size;
    void *base_region, *cur_region, *sq_region, *cq_region;
    int num_cpu;

    //Get total number of requests and reqs per thread
    total_reqs = num_producers * (total_reqs/num_producers);
    reqs_per_producer = total_reqs/num_producers;
    printf("ReqsPerProducer=%d TotalReqs:%d\n", reqs_per_producer, 4+total_reqs);

    //Allocate region & initialize queue
    LABSTOR_ERROR_HANDLE_START()
        sq_size = labstor::ipc::request_queue::GetSize(queue_depth);
        cq_size = labstor::ipc::request_map::GetSize(queue_depth);
        sq_region_size = sq_size*num_producers;
        cq_region_size = cq_size*num_producers;
        req_region_size = total_reqs*sizeof(labstor::ipc::request);
        base_region = malloc(sq_region_size + cq_region_size + req_region_size);
        cur_region = base_region;
        for(int i = 0; i < num_producers; ++i) {
            sq_region = cur_region;
            cq_region = LABSTOR_REGION_ADD(sq_size, cur_region);
            qps[i].Init(0, base_region, queue_depth, sq_region, sq_size, cq_region, cq_size);
            cur_region = LABSTOR_REGION_ADD(sq_size + cq_size, cur_region);
        }
        req_region = (labstor::ipc::request *)cur_region;
    LABSTOR_ERROR_HANDLE_END()

    printf("Starting\n");

    //Isolate this process
    labstor::ProcessAffiner mask;
    if(dedicate) {
        for(int i = num_consumers; i < mask.GetNumCPU(); ++i) {
            mask.SetCpu(i);
        }
        mask.AffineAll();
    }

    //Spam the trusted server
    omp_set_dynamic(0);
#pragma omp parallel shared(t, total_reqs, reqs_per_producer, nthreads, qps) num_threads(nthreads)
    {
        int rank = omp_get_thread_num();

        //Set this thread's affinity
        if(dedicate) {
            labstor::ProcessAffiner mask;
            mask.SetCpu(4+rank);
            mask.Affine(gettid());
            printf("Setting affinity for: %d -> %d\n", gettid(), rank);
        }

#pragma omp barrier
        //Poll and stuff
        LABSTOR_ERROR_HANDLE_START()
            if(rank < num_producers) {
                for (int i = 0; i < reqs_per_producer; ++i) {
                    int off = rank*reqs_per_producer + i;
                    req_region[off].ns_id_ = off;
                    labstor::ipc::qtok_t qtok;
                    qps[rank].Enqueue(req_region + off, qtok);
                    qps[rank].Wait<labstor::ipc::request>(qtok);
                }
                //printf("EnqueuingDone[%d]\n", rank);
            } else if(rank >= num_producers) {

                //Evenly divide queues among polling threads
                //500 producers, 10 consumers
                //Divide producers among consumers
                //P0 -> C0 .. P9 - C9
                //P10 -> C0 .. P19 - C9
                int consumer_rank = rank - num_producers;
                std::vector<int> to_poll;
                for(int i = 0; i < num_consumers; ++i) {
                    if((i % num_consumers) == consumer_rank) {
                        to_poll.emplace_back(i);
                    }
                }

                //Start polling
                int i = 0;
                t.Resume();
                while (i < total_reqs) {
                    for(int &j : to_poll) {
                        if (!qps[j].Dequeue(rq)) { continue; }
                        qps[j].Complete(rq, rq);
                        ++i;
                    }
                }
                t.Pause();
                printf("%d longer digesting!\n", rank);
            }

#pragma omp barrier
        LABSTOR_ERROR_HANDLE_END()
    }

    printf("Thrpt: %lf Kops\n", total_reqs/t.GetMsec());
}

void test_syscall(uint32_t total_reqs) {
    int fd = open("/tmp/hi.txt", O_RDWR);
    labstor::HighResMonotonicTimer t;

    printf("Testing syscalls\n");
    t.Resume();
    for(int i = 0; i < total_reqs; ++i) {
        read(fd, (void*)&fd, 0);
    }
    t.Pause();

    printf("Thrpt: %lf Kops\n", total_reqs/t.GetMsec());
}

int main(int argc, char **argv) {
    labstor::HighResMonotonicTimer t;
    produce_and_consume(1<<18, 1, 1, true, 8192);
    //test_syscall(1<<18);
    return 0;
}