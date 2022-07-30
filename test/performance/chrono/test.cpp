
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
#include <future>

int main() {
    labstor::HighResCpuTimer t[2], t_test;
    std::promise<bool> promise;
    std::future<bool> future = promise.get_future();

    for(int i = 0; i < 10000; ++i) {
        t[0].Resume();
        t_test.Resume();
        t[0].Pause();
    }

    for(int i = 0; i < 10000; ++i) {
        t[1].Resume();
        future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready;
        t[1].Pause();
    }

    printf("Thrpt: %lf Kops\n", 1000/t[0].GetMsec());
    printf("Thrpt: %lf Kops\n", 1000/t[1].GetMsec());
}