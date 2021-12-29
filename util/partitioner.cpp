//
// Created by lukemartinlogan on 12/6/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <labstor/userspace/util/partitioner.h>

int main(int argc, char **argv) {
    if(argc < 3) {
        printf("USAGE: ./partitions [PID] [core1] ... [coreN]\n");
        exit(1);
    }
    int isol_pid = atoi(argv[1]);
    int n_cpu = get_nprocs_conf();
    labstor::ProcessAffiner mask;

    //Get process partitions
    mask.SetCpu(0);
    mask.AffineAll();

    //Set process affinities
    mask.Clear();
    mask.SetCpu(1);
    mask.Affine(isol_pid);
}