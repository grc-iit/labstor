//
// Created by lukemartinlogan on 2/9/22.
//

#include "labstor/userspace/client/client.h"
#include "labmods/kernel/blkdev_table/client/blkdev_table_client.h"
#include <labmods/storage_api/mq_driver/client/mq_driver_client.h>

int main(int argc, char **argv) {
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    labstor::BlkdevTable::Client blkdev_table;
    labstor::MQDriver::Client mq_driver_;
    if(argc != 3) {
        printf("USAGE: ./mount.mq_driver [ns_key] [dev_path]\n");
        exit(1);
    }
    char *ns_key = argv[1];
    char *dev_path = argv[2];
    LABSTOR_ERROR_HANDLE_START()
    ipc_manager_ = LABSTOR_IPC_MANAGER;
    ipc_manager_->Connect();
    blkdev_table.Register();
    int dev_id = blkdev_table.RegisterBlkdev(dev_path);
    mq_driver_.Register(ns_key, dev_id);
    printf("MQDriver: Registered %s (%s) as device %d (ns_id=%d)\n", dev_path, ns_key, dev_id, mq_driver_.GetNamespaceID());
    LABSTOR_ERROR_HANDLE_END()
}