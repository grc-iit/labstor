//
// Created by lukemartinlogan on 9/12/21.
//

#include <labstor/util/singleton.h>
#include <labstor/userspace_client/client.h>
#include <labstor/userspace_client/ipc_manager.h>

int main() {
    auto ipc_manager_ = scs::Singleton<labstor::Client::IPCManager>::GetInstance();
    ipc_manager_->Connect(4);
}