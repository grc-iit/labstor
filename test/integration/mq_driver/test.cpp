//
// Created by lukemartinlogan on 12/5/21.
//
#include <labstor/userspace/client/client.h>
#include <modules/kernel/blkdev_table/client/blkdev_table_client.h>
#include <modules/kernel/mq_driver/client/mq_driver_client.h>

#include <unistd.h>

int main() {
    LABSTOR_ERROR_HANDLE_START()
        //Connect to trusted server
        LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
        ipc_manager_->Connect();

        //Register BDEV
        labstor::BlkdevTable::Client blkdev_table;
        printf("PID: %d\n", getpid());
        blkdev_table.Register();
        int dev_id = blkdev_table.RegisterBlkdev("/dev/sdb");

        //Create I/O request data
        size_t buf_size =  128*labstor::SizeType::KB;
        void *user_buf = aligned_alloc(4096, buf_size);
        size_t sector = 0;
        int hctx = 0;

        //Register MQ driver
        labstor::MQDriver::Client mq_driver;
        printf("PID: %d\n", getpid());
        mq_driver.Register();
        mq_driver.Write(dev_id, user_buf, buf_size, sector, hctx);


    LABSTOR_ERROR_HANDLE_END()
}
