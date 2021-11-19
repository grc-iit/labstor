//
// Created by lukemartinlogan on 11/2/21.
//

#include <labstor/types/module.h>
#include "simple_module.h"

SimpleModule::SimpleModule() {
    module_id_ = SIMPLE_MODULE_ID;
}
void SimpleModule::print1() {
    printf("HELLO1\n");
}
void SimpleModule::print2() {
    printf("HELLO2\n");
}
void SimpleModule::print3() {
    printf("HELLO3\n");
}
void SimpleModule::ProcessRequest(labstor::queue_pair *qp, void *request, labstor::credentials *creds) {
    printf("HERE?");
}

LABSTOR_MODULE_CONSTRUCT(SimpleModule)