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
    std::vector<bool> core_map;
    ProcessPartitioner partitioner;

    //Get process partitions
    partitioner.InitCoreMap(core_map, n_cpu);
    for(int i = 2; i < argc; ++i) {
        int core = atoi(argv[i]);
        core_map[core] = true;
    }

    //Set process affinities
    partitioner.Partition(isol_pid, core_map, n_cpu);
}