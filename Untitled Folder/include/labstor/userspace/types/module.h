//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_MODULE_H
#define LABSTOR_MODULE_H

#include <dlfcn.h>
#include <unordered_map>
#include <atomic>
#include <memory>
#include <mutex>
#include <labstor/types/basics.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
#include <labstor/userspace/util/errors.h>

#define LABSTOR_MODULE_CONSTRUCT(MODULE_NAME) \
    extern "C" {                              \
        labstor::Module* create_module() { return new MODULE_NAME(); } \
    }

namespace labstor {

class Module {
protected:
    labstor::id module_id_;
public:
    Module(labstor::id module_id) : module_id_(module_id) {}
    inline labstor::id GetModuleID() { return module_id_; }

    virtual void ReinforceRequestStats(
            labstor::ipc::request *request, size_t time_measure_ns) {};
    virtual size_t EstRequestTime(
            labstor::ipc::request *request){ return 1; };
    virtual void ProcessRequest(
            labstor::ipc::queue_pair *qp,
            labstor::ipc::request *request,
            labstor::credentials *creds) {};
    virtual void StateUpdate(labstor::Module *prior) {}
};
typedef labstor::Module* (*create_module_fn)(void);

struct ModulePath {
    std::string client;
    std::string trusted_client;
    std::string server;
};

enum class ModulePathType {
    CLIENT,
    SERVER
};

struct ModuleHandle {
    create_module_fn constructor_;
    void *handle_;
};

class ModuleTable {
private:
    std::mutex mutex_;
    std::unordered_map<labstor::id, ModuleHandle> pkg_pool_;
public:
    ModuleTable() = default;

    ModuleHandle OpenModule(std::string path, labstor::id &module_id) {
        ModuleHandle module_info;
        void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if(handle == NULL) {
            throw DLSYM_MODULE_NOT_FOUND.format(path);
        }
        labstor::create_module_fn create_module = (labstor::create_module_fn)dlsym(handle, "create_module");
        if(create_module == NULL) {
            throw DLSYM_MODULE_NO_CONSTRUCTOR.format(path);
        }
        module_id = create_module()->GetModuleID();
        module_info.constructor_ = create_module;
        module_info.handle_ = handle;
        return module_info;
    }

    void SetModuleConstructor(labstor::id module_id, labstor::ModuleHandle &module_info) {
        mutex_.lock();
        TRACEPOINT("Adding module", module_id.key, std::hash<labstor::id>()(module_id))
        if(pkg_pool_.find(module_id) != pkg_pool_.end()) {
            dlclose(pkg_pool_[module_id].handle_);
        }
        pkg_pool_[module_id] = module_info;
        mutex_.unlock();
    }

    create_module_fn GetModuleConstructor(labstor::id module_id) {
        ModuleHandle module_info;
        TRACEPOINT("GetModuleConstructor::GetModuleConstructor", module_id.key, std::hash<labstor::id>()(module_id))
        mutex_.lock();
        if(pkg_pool_.find(module_id) == pkg_pool_.end()) {
            mutex_.unlock();
            throw labstor::INVALID_MODULE_ID.format(module_id.key);
        }
        module_info = pkg_pool_[module_id];
        mutex_.unlock();
        return module_info.constructor_;
    }
};

}

#endif //LABSTOR_MODULE_H
