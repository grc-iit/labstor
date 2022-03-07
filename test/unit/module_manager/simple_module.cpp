//
// Created by lukemartinlogan on 11/2/21.
//

#include <labstor/userspace/types/module.h>
#include "simple_module.h"

void SimpleModule::print1() {
    printf("HELLO1\n");
}
void SimpleModule::print2() {
    printf("HELLO2\n");
}
void SimpleModule::print3() {
    printf("HELLO3\n");
}
void SimpleModule::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    printf("HERE?");
}

LABSTOR_MODULE_CONSTRUCT(SimpleModule, LABSTOR_SIMPLE_MODULE_ID)