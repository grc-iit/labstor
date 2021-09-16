//
// Created by lukemartinlogan on 9/12/21.
//

#include <labstor/util/singleton.h>
#include <labstor/kernel_client/kernel_client.h>

int main() {
    auto labstor_context = scs::Singleton<labstor::LabStorKernelClientContext>::GetInstance();
    labstor_context->Connect(4, 1024);
}