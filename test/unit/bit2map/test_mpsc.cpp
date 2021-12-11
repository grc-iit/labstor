//
// Created by lukemartinlogan on 12/9/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/util/timer.h>
#include <modules/kernel/ipc_test/client/ipc_test_client.h>
#include <labstor/types/data_structures/bit2map.h>

#include <unistd.h>

void PrintBit2map(labstor_bit2map_t *bit2map, int num_entries) {
    for(int i = 0; i < num_entries; ++i) {
        if(i%32 == 0) {
            printf("\n");
        }
        printf("[");
        for(int j = 0; j < 2; ++j) {
            printf("%d", labstor_bit2map_IsSet(bit2map, i, j));
        }
        printf("]");
    }
}

int main() {
    int nthreads = 8;
    int last_rank = nthreads - 1;
    int reqs_per_thread = 8192;
    int total_reqs = reqs_per_thread * (nthreads - 1);
    int total_sets = total_reqs * 2;
    labstor_bit2map_t *bit2map = (labstor_bit2map_t *)malloc(labstor_bit2map_GetSize(total_reqs));
    labstor::Timer t;
    int count = 0;
    bool basic = false;

    printf("Starting\n");
    printf("SIZE3[%d entires]: %d\n", total_reqs, labstor_bit2map_GetSize(total_reqs));

    //Spam the trusted server
    omp_set_dynamic(0);
#pragma omp parallel shared(total_sets, total_reqs, reqs_per_thread, nthreads, t, count, bit2map) num_threads(nthreads)
    {
        LABSTOR_ERROR_HANDLE_START()
        int rank = omp_get_thread_num();

        if(rank != last_rank) {
            for(int i = 0; i < reqs_per_thread; ++i) {
                int entry_idx = rank * reqs_per_thread + i;
                for(int j = 0; j < 2; ++j) {
                    while(!labstor_bit2map_TestAndSet(bit2map, entry_idx, 1<<j, 1<<j));
                }
            }
        } else {
            t.Resume();
            if(basic) {
                while (t.GetMsecFromStart() < 1000) {
                    count = 0;
                    for (int i = 0; i < total_reqs; ++i) {
                        for (int j = 0; j < 2; ++j) {
                            if (labstor_bit2map_IsSet(bit2map, i, 1<<j)) {
                                ++count;
                            }
                        }
                    }
                    if (count == total_sets) break;
                }
            }
            else {
                while (count != total_sets && t.GetMsecFromStart() < 6000) {
                    for(int i = 0; i < total_reqs; ++i) {
                        for(int j = 0; j < 2; ++j) {
                            if (labstor_bit2map_IsSet(bit2map, i, 1<<j)) {
                                labstor_bit2map_Unset(bit2map, i, 1<<j);
                                ++count;
                            }
                        }
                    }
                }
            }
        }
#pragma omp barrier
        LABSTOR_ERROR_HANDLE_END()
    }

    if(count != total_sets) {
        printf("A total of %d bits were not set\n", total_sets - count);
        //PrintBit2map(bit2map, total_reqs);
        exit(1);
    }
    printf("Success\n");
}