//
// Created by lukemartinlogan on 11/26/21.
//

#include "manual_client.h"

labstor::ipc::qtok labstor::iosched::Manual::Client::AWrite(void *buf, size_t buf_size, size_t lba, int hctx) {
    return AIO(kWriteOp, buf, buf_size, lba, hctx);
}

labstor::ipc::qtok labstor::iosched::Manual::Client::ARead(void *buf, size_t buf_size, size_t lba, int hctx) {
    return AIO(kReadOp, buf, buf_size, lba, hctx);
}

void labstor::iosched::Manual::Client::Write(void *buf, size_t buf_size, size_t lba, int hctx) {
    IO(kWriteOp, buf, buf_size, lba, hctx);
}

void labstor::iosched::Manual::Client::Read(void *buf, size_t buf_size, size_t lba, int hctx) {
    IO(kReadOp, buf, buf_size, lba, hctx);
}