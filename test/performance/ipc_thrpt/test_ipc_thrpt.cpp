//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include "labstor/userspace/client/client.h"
#include "labstor/userspace/util/timer.h"
#include "labmods/ipc_test/client/ipc_test_client.h"

#include <unistd.h>

int main(int argc, char **argv) {
    if(argc != 7) {
        printf("./server_thrpt [n_server_cores] [n_kernel_cores] [dedicated] [n_clients] [n_msgs] [batch_size]\n");
        exit(1);
    }

    printf("STARTING!\n");

    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    char *n_server_cores = argv[1];
    char *n_kernel_cores = argv[2];
    char *dedicated = argv[3];
    int n_clients = atoi(argv[4]);
    int n_msgs = atoi(argv[5]);
    int batch_size = atoi(argv[6]);
    int n_batches = n_msgs / batch_size;
    n_msgs = n_batches * batch_size;


    labstor::HighResMonotonicTimer t;
    printf("Server throughput test starting (%d clients)\n", n_clients);

    //Register client with trusted server
    labstor::IPCTest::Client client;
    ipc_manager_->Connect();

    printf("Connected?\n");

    client.GetNamespaceID();

    printf("IPC Manager Connected?\n");

    //Spam the trusted server
    omp_set_dynamic(0);
    #pragma omp parallel shared(n_server_cores, n_kernel_cores, dedicated, n_clients, n_msgs, batch_size, client, t) num_threads(n_clients)
    {
        LABSTOR_ERROR_HANDLE_START()
        int rank;
        rank = omp_get_thread_num();
        if(rank == 0) {
            t.Resume();
        }
        for (int i = 0; i < n_batches; ++i) {
            TRACEPOINT("BATCH", i)
            //printf("[tid=%d] BATCH: %d\n", rank, i);
            client.Start(batch_size);
        }
        #pragma omp barrier
        if(rank == 0) {
            t.Pause();
        }
        LABSTOR_ERROR_HANDLE_END()
    }

    printf("%s,%s,%s,%d,%d,%d,%lf,%lf\n",
           n_server_cores,
           n_kernel_cores,
           dedicated,
           n_clients,
           n_msgs,
           batch_size,
           t.GetUsec(),
           n_msgs/t.GetUsec());
}