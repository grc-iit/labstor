
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

#include <unistd.h>

void PrintBitmap(labstor_bitmap_t *bitmap, int num_bits) {
    for(int i = 0; i < num_bits; ++i) {
        if(i%64 == 0) {
            printf("\n");
        }
        printf("%d", labstor_bitmap_IsSet(bitmap, i));
    }
}

int main() {
    int nthreads = 256;
    int last_rank = nthreads - 1;
    int reqs_per_thread = 256000;
    int total_reqs = reqs_per_thread * (nthreads - 1);
    labstor_bitmap_t *bitmap = (labstor_bitmap_t *)malloc(labstor_bitmap_GetSize(total_reqs));
    labstor::HighResMonotonicTimer t;
    int count = 0;
    bool basic = false;

    printf("Starting: %d\n", total_reqs);

    //Spam the trusted server
    omp_set_dynamic(0);
#pragma omp parallel shared(total_reqs, reqs_per_thread, nthreads, t, count, bitmap) num_threads(nthreads)
    {
        LABSTOR_ERROR_HANDLE_START()
        int rank = omp_get_thread_num();

        if(rank != last_rank) {
            //Set bitmap to true
            for(int i = 0; i < reqs_per_thread; ++i) {
                int bit = rank*reqs_per_thread + i;
                        labstor_bitmap_Set(bitmap, bit);
            }
        } else {
            t.Resume();

            if(basic) {
                while (t.GetMsecFromStart() < 1000) {
                    count = 0;
                    for(int i = 0; i < total_reqs; ++i) {
                        if(labstor_bitmap_IsSet(bitmap, i)) {
                            ++count;
                        }
                    }
                    if(count == total_reqs) break;
                }
            } else {
                while (count != total_reqs && t.GetMsecFromStart() < 1000) {
                    for (int i = 0; i < total_reqs; ++i) {
                        if (labstor_bitmap_IsSet(bitmap, i)) {
                            labstor_bitmap_Unset(bitmap, i);
                            ++count;
                        }
                    }
                }
            }

            printf("Done?\n");
        }
#pragma omp barrier
        LABSTOR_ERROR_HANDLE_END()
    }

    if(count != total_reqs) {
        printf("A total of %d bits were not set\n", total_reqs - count);
        //PrintBitmap(bitmap, total_reqs);
        exit(1);
    }
    printf("Success\n");
}