//
// Created by lukemartinlogan on 12/3/21.
//

#ifndef LABSTOR_BLKDEV_TABLE_CLIENT_H
#define LABSTOR_BLKDEV_TABLE_CLIENT_H

#include <modules/kernel/blkdev_table/blkdev_table.h>
#include <labstor/constants/macros.h>
#include <labstor/constants/constants.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/client.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>

namespace labstor::BlkdevTable {

    class Client: public labstor::Module {
    private:
        LABSTOR_IPC_MANAGER_T ipc_manager_;
        uint32_t ns_id_;
    public:
        Client() : labstor::Module(BLKDEV_TABLE_MODULE_ID) {
            ipc_manager_ = LABSTOR_IPC_MANAGER;
        }
        void Register();
        int RegisterBlkdev(std::string path);
        void UnregisterBlkdev(int dev_id);
    };

}

#endif //LABSTOR_BLKDEV_TABLE_CLIENT_H
