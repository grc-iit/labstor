//
// Created by lukemartinlogan on 12/5/21.
//

#include <labstor/constants/debug.h>
#include <labmods/registrar/registrar.h>
#include <labmods/iosched/no_op/client/no_op_client.h>

void labstor::iosched::NoOp::Client::Register(const std::string &ns_key, const std::string &next_module) {
    AUTO_TRACE("")
    auto registrar = labstor::Registrar::Client();
    ns_id_ = registrar.RegisterInstance<register_request>(NO_OP_IOSCHED_MODULE_ID, ns_key, next_module);
}

labstor::ipc::qtok_t labstor::iosched::NoOp::Client::AIO(void *buf, size_t size, size_t off, labstor::GenericBlock::Ops op) {
    return labstor::ipc::qtok_t();
}

LABSTOR_MODULE_CONSTRUCT(labstor::iosched::NoOp::Client, NO_OP_IOSCHED_MODULE_ID);