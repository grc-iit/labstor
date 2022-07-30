
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