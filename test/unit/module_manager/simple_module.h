//
// Created by lukemartinlogan on 11/2/21.
//

#ifndef LABSTOR_SIMPLE_MODULE_H
#define LABSTOR_SIMPLE_MODULE_H

#include <labstor/userspace/types/module.h>

class SimpleModule : public labstor::Module {
public:
    SimpleModule() : Module("SIMPLE_MODULE") {}
    void print1();
    void print2();
    void print3();
    void ProcessRequest(labstor::ipc::queue_pair qp, labstor::ipc::request *request, labstor::credentials *creds);
};

#endif //LABSTOR_SIMPLE_MODULE_H
