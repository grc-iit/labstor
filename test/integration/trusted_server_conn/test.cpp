//
// Created by lukemartinlogan on 11/26/21.
//

#include <omp.h>
#include <labstor/userspace/client/client.h>
#include <labmods/dummy/client/dummy_client.h>

#include <unistd.h>

int main(int argc, char **argv) {
    if(argc != 3) {
        printf("./trusted_server_conn [n_clients] [n_msgs\n");
        exit(1);
    }

    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    int n_clients = atoi(argv[1]);
    int n_msgs = atoi(argv[2]);
    labstor::test::Dummy::Client client;
    labstor::HighResMonotonicTimer t;

    //Register client with trusted server
    LABSTOR_ERROR_HANDLE_START()
    ipc_manager_->Connect();
    client.Register();
    LABSTOR_ERROR_HANDLE_END()

    //Spam the trusted server
    omp_set_dynamic(0);
    #pragma omp parallel shared(n_clients, n_msgs, client, t) num_threads(n_clients)
    {
        t.Resume();
        LABSTOR_ERROR_HANDLE_START()
        for (int i = 0; i < n_msgs; ++i) {
            client.GetValue();
        }
        t.Pause();
        printf("Thread[%d] Completed\n", omp_get_thread_num());
        #pragma omp barrier
        LABSTOR_ERROR_HANDLE_END()
    }

    printf("Completed test: %lf\n", t.GetUsec());
}