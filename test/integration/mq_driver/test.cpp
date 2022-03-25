//
// Created by lukemartinlogan on 12/5/21.
//


#include <labstor/userspace/client/client.h>
#include <labmods/kernel/blkdev_table/client/blkdev_table_client.h>
#include <labmods/storage_api/mq_driver/client/mq_driver_client.h>
#include <unistd.h>

bool verify_buf(int value, char *buffer, size_t buf_size, const char *path) {
    for(int i = 0; i < buf_size; ++i) {
        if(buffer[i] != value) {
            printf("%s[%d] = %d, but should be %d\n", path, i, buffer[i], value);
            return false;
        }
    }
    printf("All bytes were equal to %d\n", value);
    return true;
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
    int ret = pread(fd, buffer, buf_size, sector*512);
    if(ret < 0) {
        perror("Failed to pread\n");
        exit(1);
    }
    verify_buf(value, buffer, buf_size, path);

    close(fd);
    free(buffer);
}

int main(int argc, char **argv) {
    if(argc < 3) {
        printf("USAGE: ./test_mq_driver_exec [path] [num_ios]");
        exit(1);
    }
    char *path = argv[1];
    int num_ios = atoi(argv[2]);

    LABSTOR_ERROR_HANDLE_START()
        //Connect to trusted server
        LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
        ipc_manager_->Connect();

        //Register BDEV
        labstor::BlkdevTable::Client blkdev_table;
        blkdev_table.Register();
        int dev_id = blkdev_table.RegisterBlkdev(path);
        printf("HERE1, %d\n", dev_id);

        //Create I/O request data
        int nonce = 12;
        size_t buf_size =  4*labstor::SizeType::KB;
        void *user_buf = aligned_alloc(4096, buf_size);
        size_t sector = 0;
        int hctx = 0;
        memset(user_buf, nonce, buf_size);

        //Register MQ driver
        labstor::MQDriver::Client mq_driver;
        mq_driver.Register();

        //Write to device
        for(int i = 0; i < num_ios; ++i) {
            printf("CURRENT: %d\n", i);
            memset(user_buf, nonce, buf_size);
            mq_driver.Write(dev_id, user_buf, buf_size, sector, hctx);
            memset(user_buf, 0, buf_size);

            //Read from device
            mq_driver.Read(dev_id, user_buf, buf_size, sector, hctx);

            //Verify I/O completed
            if(!verify_buf(nonce, (char*)user_buf, buf_size, path)) {
                exit(1);
            }
        }
    LABSTOR_ERROR_HANDLE_END()
}
