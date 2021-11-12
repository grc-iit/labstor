//
// Created by lukemartinlogan on 9/12/21.
//

#include <labstor/util/singleton.h>
#include <labstor/kernel_client/kernel_client.h>

int main() {
    auto kernel_client_ = scs::Singleton<labstor::Kernel::NetlinkClient>::GetInstance();
    kernel_client_->Connect();
}