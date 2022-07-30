
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

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/util/timer.h>
#include <labmods/ipc_test/client/ipc_test_client.h>
#include "labstor/types/data_structures/bit2map.h"

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

int GetNumCompleted(std::vector<int> &was_dequeued) {
    int count = 0;
    for(int i = 0; i < was_dequeued.size(); ++i) {
        if(was_dequeued[i]) {
            ++count;
        }
    }
    return count;
}

void produce_consume(int num_producers, int num_consumers, int total_reqs) {
    int reqs_per_thread, nthreads;
    labstor_bit2map_t *bit2map = (labstor_bit2map_t *)malloc(labstor_bit2map_GetSize(total_reqs));
    std::vector<int> being_set, was_set, was_dequeued;
    labstor_bit2map_Init(bit2map, total_reqs);

    //Some ints
    nthreads = num_producers + num_consumers;
    total_reqs = (total_reqs/num_producers)*num_producers;
    reqs_per_thread = total_reqs/num_producers;

    //Initialize trackers
    being_set.resize(total_reqs);
    was_set.resize(total_reqs);
    was_dequeued.resize(total_reqs);
    for(int i = 0; i < total_reqs; ++i) {
        being_set[i] = 0;
        was_set[i] = 0;
        was_dequeued[i] = 0;
    }

//Spam the trusted server
    omp_set_dynamic(0);
#pragma omp parallel shared(total_reqs, reqs_per_thread, nthreads, bit2map) num_threads(nthreads)
    {
        LABSTOR_ERROR_HANDLE_START()
            int rank = omp_get_thread_num();

            if(rank < num_producers) {
                for(int i = 0; i < reqs_per_thread; ++i) {
                    int entry_idx = rank * reqs_per_thread + i;
                    ++being_set[i];
                    if(labstor_bit2map_BeginModify(bit2map, entry_idx)) {
                        ++was_set[i];
                        /*printf("[A]VALID: %d, BEING_SET: %d\n",
                               labstor_bit2map_IsSet(bit2map, entry_idx,LABSTOR_BIT2MAP_VALID),
                               labstor_bit2map_IsSet(bit2map, entry_idx,LABSTOR_BIT2MAP_BEING_SET));*/
                        labstor_bit2map_CommitModify(bit2map, entry_idx);
                        /*printf("[B]VALID: %d, BEING_SET: %d\n",
                               labstor_bit2map_IsSet(bit2map, entry_idx,LABSTOR_BIT2MAP_VALID),
                               labstor_bit2map_IsSet(bit2map, entry_idx,LABSTOR_BIT2MAP_BEING_SET));*/
                    } else {
                        printf("Some bits are set that aren't supposed to be\n");
                    }
                }
                printf("FinishedProducing[%d]\n", rank);
            } else {
                labstor::HighResMonotonicTimer t;
                t.Resume();
                int count = 0;
                while (count != total_reqs && t.GetMsecFromStart() < 6000) {
                    for(int i = 0; i < total_reqs; ++i) {
                        if(labstor_bit2map_BeginRemove(bit2map, i)) {
                            ++was_dequeued[i];
                            labstor_bit2map_CommitRemove(bit2map, i);
                        }
                    }
                    count = GetNumCompleted(was_dequeued);
                }
                printf("FinishedConsuming[%d]\n", rank);
            }
#pragma omp barrier
        LABSTOR_ERROR_HANDLE_END()
    }

    int count = GetNumCompleted(was_dequeued);
    printf("%d / %lu were dequeued\n", count, was_dequeued.size());
}

int main() {
    produce_consume(32, 32, 1<<24);
}