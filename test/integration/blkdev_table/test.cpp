//
// Created by lukemartinlogan on 11/26/21.
//

#include <labstor/userspace/client/client.h>
#include <modules/kernel/blkdev_table/client/blkdev_table_client.h>

#include <unistd.h>

int main() {
    LABSTOR_ERROR_HANDLE_START()

    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
    labstor::BlkdevTable::Client client;
    printf("PID: %d\n", getpid());
    ipc_manager_->Connect();
    client.Register();
    client.RegisterBlkdev("/dev/sda");

    LABSTOR_ERROR_HANDLE_END()
}