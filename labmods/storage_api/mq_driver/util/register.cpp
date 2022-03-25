//
// Created by lukemartinlogan on 2/9/22.
//

#include "labstor/userspace/client/client.h"
#include "labmods/kernel/blkdev_table/client/blkdev_table_client.h"
#include <labmods/storage_api/mq_driver/client/mq_driver_client.h>

int main(int argc, char **argv) {
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    labstor::MQDriver::Client mq_driver_;
    if(argc != 1) {
        printf("USAGE: ./register\n");
        exit(1);
    }
    LABSTOR_ERROR_HANDLE_START()
    ipc_manager_ = LABSTOR_IPC_MANAGER;
    ipc_manager_->Connect();
    mq_driver_.Register();
    LABSTOR_ERROR_HANDLE_END()
}