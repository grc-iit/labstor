//
// Created by lukemartinlogan on 12/5/21.
//
#include <labstor/userspace/client/client.h>
#include <modules/kernel/blkdev_table/client/blkdev_table_client.h>
#include <modules/kernel/mq_driver/client/mq_driver_client.h>

#include <unistd.h>

void verify_buf(int value, char *buffer, size_t buf_size, const char *path) {
    for(int i = 0; i < buf_size; ++i) {
        if(buffer[i] != value) {
            printf("%s[%d] = %d, but should be %d", path, i, buffer[i], value);
            return;
        }
    }
    printf("All bytes were equal to %d\n", value);
}

void direct_read(int value, size_t sector, size_t buf_size, const char *path) {
    char *buffer = (char*)aligned_alloc(4096, buf_size);
    if(buffer == NULL) {
        printf("Could not allocate memory (direct_read)\n");
        exit(1);
    }
    memset(buffer, 0, buf_size);

    int fd = open(path, O_DIRECT | O_RDONLY);
    if(fd < 0) {
        printf("Could not open file (direct_read) (%s)\n", path);
        exit(1);
    }
    pread(fd, buffer, buf_size, sector*512);
    verify_buf(value, buffer, buf_size, path);

    close(fd);
    free(buffer);
}

int main() {
    LABSTOR_ERROR_HANDLE_START()
        //Connect to trusted server
        LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
        ipc_manager_->Connect();

        //Register BDEV
        std::string path = "/dev/nvme0n1";
        labstor::BlkdevTable::Client blkdev_table;
        printf("PID: %d\n", getpid());
        blkdev_table.Register();
        int dev_id = blkdev_table.RegisterBlkdev(path);

        //Create I/O request data
        int nonce = 12;
        size_t buf_size =  128*labstor::SizeType::KB;
        void *user_buf = aligned_alloc(4096, buf_size);
        memset(user_buf, nonce, buf_size);
        size_t sector = 0;
        int hctx = 0;

        //Register MQ driver
        labstor::MQDriver::Client mq_driver;
        printf("PID: %d\n", getpid());
        mq_driver.Register();

        //Write to device
        for(int i = 0; i < 256; ++i) {
            mq_driver.Write(dev_id, user_buf, buf_size, sector, hctx);
            memset(user_buf, 0, buf_size);

            //Read from device
            mq_driver.Read(dev_id, user_buf, buf_size, sector, hctx);
        }

        //Verify I/O completed
        verify_buf(nonce, (char*)user_buf, buf_size, path.c_str());

    LABSTOR_ERROR_HANDLE_END()
}
