//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_REGISTRAR_H
#define LABSTOR_REGISTRAR_H

#include <labstor/constants/constants.h>
#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>

namespace labstor::Registrar {

enum class Ops {
    kRegister,
    kGetNamespaceId
};

struct register_request : labstor::ipc::request {
    labstor::id module_id_;
    labstor::id key_;
    void Start(const std::string &module_id, const std::string &key) {
        ns_id_ = LABSTOR_REGISTRAR_ID;
        op_ = static_cast<int>(Ops::kRegister);
        module_id_.copy(module_id);
        key_.copy(key);
    }
    void Complete(uint32_t ns_id) {
        ns_id_ = ns_id;
    }
};

struct namespace_id_request : labstor::ipc::request {
    labstor::id key_;
    void Start(const std::string &key) {
        ns_id_ = LABSTOR_REGISTRAR_ID;
        op_ = static_cast<int>(Ops::kGetNamespaceId);
        key_.copy(key);
    }
    void Complete(uint32_t ns_id) {
        ns_id_ = ns_id;
    }
};

}

#endif //LABSTOR_REGISTRAR_H
