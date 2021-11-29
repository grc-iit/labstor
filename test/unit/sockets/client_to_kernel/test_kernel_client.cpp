//
// Created by lukemartinlogan on 9/12/21.
//
#include <labstor/kernel_client/macros.h>
#include <labstor/kernel_client/kernel_client.h>

int main() {
    auto kernel_client_ = LABSTOR_KERNEL_CLIENT;
    kernel_client_->Connect();
}