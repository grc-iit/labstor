//
// Created by lukemartinlogan on 11/2/21.
//

#ifndef LABSTOR_SIMPLE_MODULE_H
#define LABSTOR_SIMPLE_MODULE_H

#include <labstor/types/module.h>

const labstor::id SIMPLE_MODULE_ID = "SIMPLE_MODULE";

class SimpleModule : public labstor::Module {
public:
    SimpleModule();
    void print1();
    void print2();
    void print3();
    void ProcessRequest(labstor::ipc::queue_pair *qp, void *request, labstor::credentials *creds);
};

#endif //LABSTOR_SIMPLE_MODULE_H
