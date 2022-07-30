
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
#include "labstor/types/data_structures/c/shmem_queue_pair.h"
#include <labstor/types/thread_local.h>
#include <labstor/userspace/util/partitioner.h>

#include <sys/sysinfo.h>
#include <fcntl.h>
#include <unistd.h>

void test_latency() {
    int num_cpu = get_nprocs_conf();
    int *shmem = (int*)calloc(1,2048);
    labstor::HighResMonotonicTimer t;

    //Spam the trusted server
    omp_set_dynamic(0);
#pragma omp parallel shared(t) num_threads(2)
    {
        int rank = omp_get_thread_num();

        //Set this thread's affinity
        cpu_set_t cpus[num_cpu];
        CPU_ZERO(cpus);
        CPU_SET(1+rank*8, cpus);
        sched_setaffinity(gettid(), num_cpu, cpus);
        printf("Setting affinity for: %d -> %d\n", gettid(), 1+rank*8);

#pragma omp barrier
        //Poll and stuff
        LABSTOR_ERROR_HANDLE_START()
            if(rank == 1) {
                while(shmem[24] == 0) {
                    continue;
                }
                printf("END: %lf\n", t.GetUsFromEpoch());
            }
            if(rank == 0) {
                shmem[24] = 1;
                printf("START: %lf\n", t.GetUsFromEpoch());
            }
        LABSTOR_ERROR_HANDLE_END()
#pragma omp barrier
    }
}

int main(int argc, char **argv) {
    test_latency();
    return 0;
}