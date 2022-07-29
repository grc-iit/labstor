
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