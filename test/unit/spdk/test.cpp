
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

#include <labmods/spdk/spdk.h>

void help() {
    printf("USAGE: ./test [opt (1/2)] [trid (case 2)] [ns_id (case 2)] [num_qps (case 2)]\n");
    exit(1);
}

int main(int argc, char **argv) {
    labstor::SPDK::Context lib;
    char *trid = nullptr;
    int ns_id = 0, num_qps = 0;
    int opt = 0;
    if(argc == 1) {
        help();
    }
    opt = atoi(argv[1]);
    if(opt == 2 && argc != 5) {
        help();
    }
    if(opt == 2) {
        trid = argv[2];
        ns_id = atoi(argv[3]);
        num_qps = atoi(argv[4]);
    }

    printf("OPT: %d\n", opt);

    switch(opt) {
        //List NVME devices
        case 1: {
            lib.Init();
            lib.Probe(true);
            break;
        }

        //Allocate queue pairs
        case 2: {
            lib.Init();
            lib.Probe(false);
            if(!lib.SelectDevice(trid, ns_id)) {
                printf("TRID: %s\n", trid);
                printf("NSID: %d\n", ns_id);
                printf("Failed to find device\n");
                exit(1);
            }
            break;
        }

        default: {
            help();
        }
    }
}