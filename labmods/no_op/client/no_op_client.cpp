//
// Created by lukemartinlogan on 12/5/21.
//

#include "labstor/constants/debug.h"
#include "labmods/registrar/registrar.h"
#include "no_op_client.h"

void labstor::iosched::NoOp::Client::Register(YAML::Node config) {
    AUTO_TRACE("")
    ns_id_ = LABSTOR_REGISTRAR->RegisterInstance(NO_OP_IOSCHED_MODULE_ID, config["labmod_uuid"].as<std::string>());
    LABSTOR_REGISTRAR->InitializeInstance<register_request>(ns_id_, config["next"].as<std::string>());
}

labstor::ipc::qtok_t labstor::iosched::NoOp::Client::AIO(void *buf, size_t size, size_t off, labstor::GenericBlock::Ops op) {
    return labstor::ipc::qtok_t();
}

LABSTOR_MODULE_CONSTRUCT(labstor::iosched::NoOp::Client, NO_OP_IOSCHED_MODULE_ID);