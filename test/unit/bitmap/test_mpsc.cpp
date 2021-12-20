//
// Created by lukemartinlogan on 12/9/21.
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