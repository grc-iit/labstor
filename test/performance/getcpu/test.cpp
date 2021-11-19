//
// Created by lukemartinlogan on 11/13/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <labstor/util/timer.h>
#include <unistd.h>

int regular_fun(int a, int b) {
    return a + b;
}

int main() {
    uint32_t rep = 100000;
    labstor::Timer t;
    double thrpt[10];

    //Increment
    t.Reset();
    int num = 0;
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        ++num;
    }
    t.Pause();
    thrpt[0] = rep / t.GetSec();

    //Getcpu
    t.Reset();
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        sched_getcpu();
    }
    t.Pause();
    thrpt[1] = rep / t.GetSec();

    //Function call
    t.Reset();
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        regular_fun(5, 10);
    }
    t.Pause();
    thrpt[2] = rep / t.GetSec();

    //Getpid
    t.Reset();
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        getpid();
    }
    t.Pause();
    thrpt[3] = rep / t.GetSec();

    //Gettid
    t.Reset();
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        gettid();
    }
    t.Pause();
    thrpt[4] = rep / t.GetSec();

    //Get n nprocessors
    t.Reset();
    t.Resume();
    for(int i = 0; i < rep; ++i) {
        get_nprocs_conf();
    }
    t.Pause();
    thrpt[5] = rep / t.GetSec();


    printf("Increment Thrpt: %lf\n", 1.0);
    printf("Getcpu Thrpt: %lf\n", thrpt[1] / thrpt[0]);
    printf("Function Call Thrpt: %lf\n", thrpt[2] / thrpt[0]);
    printf("GetPID Thrpt: %lf\n", thrpt[3] / thrpt[0]);
    printf("GetTID Thrpt: %lf\n", thrpt[4] / thrpt[0]);
    printf("GetNPROCS Thrpt: %lf\n", thrpt[5] / thrpt[0]);
}