
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

#include <labmods/spdk/client/spdk_client.h>

void help() {
    printf("USAGE: ./test [opt (1/2)] [trid (case 2)] [ns_id (case 2)] [num_qps (case 2)]\n");
    exit(1);
}

void verify_buf(char *buf, int size, int nonce, int test) {
    for(int i = 0; i < size; ++i) {
        if(buf[i] != nonce) {
            printf("Buffer was not correct (case %d)\n", test);
            exit(1);
        }
    }
    printf("SUCCESS (%d)!\n", test);
}

int main(int argc, char **argv) {
    labstor::ipc::qtok_t qtok;
    labstor::SPDK::Client client;
    if(argc < 3) {
        printf("USAGE: ./test_spdk_module [traddr] [ns_id]\n");
        exit(1);
    }

    LABSTOR_ERROR_HANDLE_START()

    //Connect to trusted server & initialize module
    LABSTOR_IPC_MANAGER->Connect();
    client.Init(argv[1], atoi(argv[2]));

    //Create request
    int size = 4*(1<<10);
    void *mem = client.Alloc(size);
    memset(mem, 8, size);

    //Write 4KB of data
    qtok = client.AIO(labstor::SPDK::Ops::kWrite, mem, size, 0);
    LABSTOR_IPC_MANAGER->WaitFree(qtok);

    //Read 4KB of data
    memset(mem, 0, size);
    qtok = client.AIO(labstor::SPDK::Ops::kRead, mem, size, 0);
    LABSTOR_IPC_MANAGER->WaitFree(qtok);

    //Make sure buffer is filled with 8s
    verify_buf((char*)mem, size, 8, 1);

    //Free SHMEM
    client.Free(mem);

    LABSTOR_ERROR_HANDLE_END()
}