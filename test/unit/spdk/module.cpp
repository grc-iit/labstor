//
// Created by lukemartinlogan on 2/27/22.
//

#include <modules/storage_api/spdk/client/spdk_client.h>

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