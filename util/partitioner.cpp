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