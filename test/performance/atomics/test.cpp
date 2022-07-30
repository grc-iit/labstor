
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
#include <labstor/userspace/util/timer.h>

const int hi = 124;

void test_atomics(int total_reqs) {
    labstor::HighResMonotonicTimer t[4];
    uint32_t number = 0;

    t[0].Resume();
    for(int i = 0; i < total_reqs; ++i) {
        __atomic_compare_exchange_n(&number, &number, hi+1, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    }
    t[0].Pause();

    t[1].Resume();
    for(int i = 0; i < total_reqs; ++i) {
        __atomic_fetch_add(&number, 1, __ATOMIC_RELAXED);
    }
    t[1].Pause();

    t[2].Resume();
    for(int i = 0; i < total_reqs; ++i) {
        __atomic_fetch_and(&number, 1, __ATOMIC_RELAXED);
    }
    t[2].Pause();

    t[3].Resume();
    for(int i = 0; i < total_reqs; ++i) {
        __atomic_fetch_or(&number, 1, __ATOMIC_RELAXED);
    }
    t[3].Pause();

    printf("cmpx=%lf, add=%lf, and=%lf, or=%lf\n",
           total_reqs/t[0].GetMsec(),
           total_reqs/t[1].GetMsec(),
           total_reqs/t[2].GetMsec(),
           total_reqs/t[3].GetMsec());
}

int main() {
    test_atomics(10000000);
}