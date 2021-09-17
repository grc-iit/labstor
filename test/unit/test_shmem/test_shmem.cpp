//
// Created by lukemartinlogan on 9/12/21.
//

#include <labstor/util/singleton.h>
#include <labstor/kernel_client/kernel_client.h>

int main() {
    auto labstor_kernel_context = scs::Singleton<labstor::LabStorKernelClientContext>::GetInstance();
    labstor_kernel_context->Connect();


}