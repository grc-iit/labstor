//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_REGISTRAR_H
#define LABSTOR_REGISTRAR_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>

namespace labstor::Registrar {

enum class Ops {
    kRegister,
    kGetNamespaceId
};

struct register_submit_request : labstor::ipc::request {
    labstor::id module_id_;
    labstor::id key_;

    void Init(const std::string &module_id, const std::string &key) {
        ns_id_ = 0;
        op_ = static_cast<int>(Ops::kRegister);
        module_id_.copy(module_id);
        key_.copy(key);
    }
};

struct register_complete_request : labstor::ipc::request {
    void Init(uint32_t value) {
        ns_id_ = value;
    }
};

struct namespace_id_submit_request : labstor::ipc::request {
    labstor::id key_;

    void Init(const std::string &key) {
        ns_id_ = 0;
        op_ = static_cast<int>(Ops::kGetNamespaceId);
        key_.copy(key);
    }
};

struct namespace_id_complete_request : labstor::ipc::request {
    void Init(uint32_t ns_id) {
        ns_id_ = ns_id;
    }
};

}

#endif //LABSTOR_REGISTRAR_H
