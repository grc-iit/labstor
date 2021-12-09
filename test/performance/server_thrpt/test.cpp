//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/util/timer.h>
#include <modules/kernel/ipc_test/client/ipc_test_client.h>

#include <unistd.h>

int main(int argc, char **argv) {
    if(argc != 7) {
        printf("./server_thrpt [n_server_cores] [oversubscribe] [n_kernel_cores] [dedicated] [n_clients] [n_msgs]\n");
        exit(1);
    }

    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    char *n_server_cores = argv[1];
    char *server_core_mult = argv[2];
    char *n_kernel_cores = argv[3];
    char *dedicated = argv[4];
    int n_clients = atoi(argv[5]);
    int n_msgs = atoi(argv[6]);
    labstor::IPCTest::Client client;
    labstor::Timer t;
    printf("Server throughput test starting (%d clients)\n", n_clients);

    //Register client with trusted server
    LABSTOR_ERROR_HANDLE_START()
    ipc_manager_->Connect();
    client.Register();
    LABSTOR_ERROR_HANDLE_END()

    //Spam the trusted server
    omp_set_dynamic(0);
    #pragma omp parallel shared(n_server_cores, server_core_mult, n_kernel_cores, dedicated, n_clients, n_msgs, client, t) num_threads(n_clients)
    {
        LABSTOR_ERROR_HANDLE_START()
        int rank;
        rank = omp_get_thread_num();
        if(rank == 0) {
            t.Resume();
        }
        for (int i = 0; i < n_msgs; ++i) {
            client.Start();
        }
        #pragma omp barrier
        if(rank == 0) {
            t.Pause();
        }
        LABSTOR_ERROR_HANDLE_END()
    }

    printf("%s,%s,%s,%s,%d,%d,%lf,%lf\n",
           n_server_cores,
           server_core_mult,
           n_kernel_cores,
           dedicated,
           n_clients,
           n_msgs,
           t.GetUsec(),
           n_msgs/t.GetUsec());
}