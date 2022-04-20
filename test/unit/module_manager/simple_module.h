//
// Created by lukemartinlogan on 11/2/21.
//

#ifndef LABSTOR_SIMPLE_MODULE_H
#define LABSTOR_SIMPLE_MODULE_H

#include <labstor/userspace/types/module.h>

#define LABSTOR_SIMPLE_MODULE_ID "SIMPLE_MODULE"

class SimpleModule : public labstor::Module {
public:
    SimpleModule() : Module(LABSTOR_SIMPLE_MODULE_ID) {}
    bool Initialize(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) { return true; }
    void print1();
    void print2();
    void print3();
    bool ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds);
};

#endif //LABSTOR_SIMPLE_MODULE_H
