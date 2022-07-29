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

#include <stdio.h>
#include <stdlib.h>
#include <labstor/userspace/util/partitioner.h>

int main(int argc, char **argv) {
    if(argc < 3) {
        printf("USAGE: ./partitions [PID] [core1] ... [coreN]\n");
        printf("PID: either an integer indicating the pid to affine, or the string \"all\" indicating to affine all processes\n");
        exit(1);
    }
    int pid;
    int n_cpu = argc - 2;
    labstor::ProcessAffiner mask;

    //Get CPU set
    for(int i = 0; i < n_cpu; ++i) {
        int cpu = atoi(argv[i+2]);
        mask.SetCpu(cpu);
    }

    //Determine the PIDs being set
    if(std::string(argv[1]) == "all") {
        mask.AffineAll();
    } else {
        pid = atoi(argv[1]);
        mask.Affine(pid);
    }
}