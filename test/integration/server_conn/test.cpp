//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <modules/kernel/ipc_test/client/ipc_test_client.h>

#include <unistd.h>

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("./server_conn [n_clients] [n_msgs\n");
        exit(1);
    }

    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    int n_clients = atoi(argv[1]);
    int n_msgs = atoi(argv[2]);
    labstor::IPCTest::Client client;
    printf("Server connection test starting (%d clients)\n", n_clients);

    //Register client with trusted server
    LABSTOR_ERROR_HANDLE_START()
    ipc_manager_->Connect();
    client.Register();
    LABSTOR_ERROR_HANDLE_END()

    printf("Registered\n");

    //Spam the trusted server
    omp_set_dynamic(0);
    #pragma omp parallel shared(n_clients, n_msgs, client) num_threads(n_clients)
    {
        LABSTOR_ERROR_HANDLE_START()
        for (int i = 0; i < n_msgs; ++i) {
            client.Start();
        }
        #pragma omp barrier
        LABSTOR_ERROR_HANDLE_END()
    }

    printf("Completed test\n");
}