//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/constants/debug.h>
#include <modules/registrar/registrar.h>
#include <modules/iosched/no_op/client/no_op_client.h>

void labstor::iosched::NoOp::Client::Register(const std::string &ns_key, const std::string &dev_name) {
    AUTO_TRACE("")
}

int labstor::iosched::NoOp::Client::GetNamespaceID() {
}

LABSTOR_MODULE_CONSTRUCT(labstor::iosched::NoOp::Client, NO_OP_IOSCHED_MODULE_ID);