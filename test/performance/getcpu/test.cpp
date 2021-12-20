//
// Created by lukemartinlogan on 11/13/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <labstor/userspace/util/timer.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int regular_fun(int a, int b) {
    return a + b;
}

int main() {
    uint32_t rep = 100000;
    labstor::HighResMonotonicTimer t;
    double thrpt[10];

    //Increment
    t.Reset();
    int num = 0;
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        ++num;
    }
    t.Pause();
    thrpt[0] = rep / t.GetMsec();

    //Getcpu
    t.Reset();
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        sched_getcpu();
    }
    t.Pause();
    thrpt[1] = rep / t.GetMsec();

    //Function call
    t.Reset();
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        regular_fun(5, 10);
    }
    t.Pause();
    thrpt[2] = rep / t.GetMsec();

    //Getpid
    t.Reset();
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        getpid();
    }
    t.Pause();
    thrpt[3] = rep / t.GetMsec();

    //Gettid
    t.Reset();
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        gettid();
    }
    t.Pause();
    thrpt[4] = rep / t.GetMsec();

    //Get n nprocessors
    t.Reset();
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        get_nprocs_conf();
    }
    t.Pause();
    thrpt[5] = rep / t.GetMsec();

    //stat
    int fd = open("/tmp/hi.txt", O_RDWR);
    t.Reset();
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        struct stat stats;
        fstat(fd, &stats);
    }
    t.Pause();
    thrpt[6] = rep/ t.GetMsec();


    printf("Increment Thrpt: %lf KOps\n", thrpt[0]);
    printf("Getcpu Thrpt: %lf KOps\n", thrpt[1]);
    printf("Function Call Thrpt: %lf KOps\n", thrpt[2]);
    printf("GetPID Thrpt: %lf KOps\n", thrpt[3]);
    printf("GetTID Thrpt: %lf KOps\n", thrpt[4]);
    printf("GetNPROCS Thrpt: %lf KOps\n", thrpt[5]);
    printf("Stat Thrpt: %lf KOps\n", thrpt[6]);
}