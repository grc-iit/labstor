//
// Created by lukemartinlogan on 11/26/21.
//

#ifndef LABSTOR_REGISTRAR_H
#define LABSTOR_REGISTRAR_H

#include <labstor/constants/constants.h>
#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>

#define LABSTOR_REGISTRAR_MODULE_ID "Registrar"

#define UPGRADE_CENTRALIZED 1
#define UPGRADE_DECENTRALIZED 2

namespace labstor::Registrar {

enum class Ops {
    kRegister,
    kGetModulePath,
    kGetNamespaceId,
    kPushUpgrade,
    kTerminate,
};

struct register_request : labstor::ipc::request {
    labstor::id module_id_;
    labstor::id key_;
    void ConstructModuleStart(const std::string &module_id, const std::string &key) {
        ns_id_ = LABSTOR_REGISTRAR_ID;
        op_ = static_cast<int>(Ops::kRegister);
        module_id_.copy(module_id);
        key_.copy(key);
    }
    void ConstructModuleEnd(uint32_t ns_id) {
        ns_id_ = ns_id;
    }
};

struct namespace_id_request : labstor::ipc::request {
    labstor::id key_;
    void GetNamespaceIDStart(const std::string &key) {
        ns_id_ = LABSTOR_REGISTRAR_ID;
        op_ = static_cast<int>(Ops::kGetNamespaceId);
        key_.copy(key);
    }
    void GetNamespaceIDEnd(uint32_t ns_id, uint32_t code) {
        code_ = code;
        ns_id_ = ns_id;
    }
};

struct module_path_request : labstor::ipc::request {
    int module_ns_id_;
    char module_path_[];
    void GetModulePathStart(int ns_id) {
        ns_id_ = LABSTOR_REGISTRAR_ID;
        module_ns_id_ = ns_id;
        op_ = static_cast<int>(Ops::kGetModulePath);
    }
    void GetModulePathEnd(const std::string &module_path, uint32_t code) {
        code_ = code;
        strcpy(module_path_, module_path.c_str());
    }
    std::string GetModulePath() {
        return {module_path_};
    }
};

struct upgrade_request : labstor::ipc::request {
    labstor::id key_;
    void PushUpgradeStart(const std::string &key, int flags) {
        ns_id_ = LABSTOR_REGISTRAR_ID;
        op_ = static_cast<int>(Ops::kPushUpgrade);
        key_.copy(key);
    }
    void PushUpgradeEnd() {
        SetCode(LABSTOR_REQUEST_SUCCESS);
    }
};

struct terminate_request : labstor::ipc::request {
    void TerminateStart() {
        ns_id_ = LABSTOR_REGISTRAR_ID;
        op_ = static_cast<int>(Ops::kTerminate);
    }
    void TerminateEnd() {}
};

}

#endif //LABSTOR_REGISTRAR_H