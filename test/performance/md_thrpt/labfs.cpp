
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

#include <unordered_map>
#include "labstor/userspace/util/timer.h"

int main(int argc, char **argv) {
    std::unordered_map<std::string, int> map1;
    std::unordered_map<int,int> map2;

    //File create
    labstor::HighResMonotonicTimer t;
    t.Resume();
    for(int i = 0; i < 100000; ++i) {
        map1.emplace(std::string("hello") + std::to_string(i), i);
        map2.emplace(i,i+1);
        for(int j = 0; j<3; ++j) { malloc(1); }
    }
    t.Pause();

    printf("ops/sec: %lf\n", 100000/t.GetSec());
}