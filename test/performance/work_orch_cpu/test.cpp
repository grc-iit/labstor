
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

//Each client performs 1GB of I/O
//Clients are striped evenly among workers

#include <cstdlib>
#include <cstdio>
#include <labstor/userspace/util/timer.h>
#include <omp.h>

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("USAGE: ./work_orch_cpu [nworkers] [nclients]");
        exit(1);
    }
    int num_workers = atoi(argv[1]);
    int num_clients = atoi(argv[2]);

    labstor::HighResMonotonicTimer t;

    t.Resume();
    omp_set_dynamic(0);
#pragma omp parallel shared(t) num_threads(num_workers)
    {
    }
    t.Pause();
}