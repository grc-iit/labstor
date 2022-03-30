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
#include "labstor/types/data_structures/c/shmem_queue_pair.h"
#include <labstor/userspace/util/errors.h>

#define LABSTOR_MODULE_CONSTRUCT(MODULE_NAME, MODULE_ID) \
    extern "C" {                              \
        labstor::Module* create_module() { return new MODULE_NAME(); } \
        labstor::id get_module_id(void) { return labstor::id(MODULE_ID); } \
    }

namespace labstor {

class Module {
protected:
    labstor::id module_id_;
    uint32_t ns_id_;
public:
    Module(labstor::id module_id) : module_id_(module_id), ns_id_(0) {}
    inline labstor::id GetModuleID() { return module_id_; }
    void SetNamespaceID(uint32_t ns_id) { ns_id_ = ns_id; }
    uint32_t GetNamespaceID() { return ns_id_; }

    virtual void Initialize(labstor::ipc::request *rq) = 0;
    virtual void Initialize(int ns_id) {}
    virtual void ReinforceCpuTime(
            labstor::ipc::request *request, size_t time_measure_ns) {};
    virtual void ReinforceIOTime(
            labstor::ipc::request *request, size_t time_measure_ns) {};
    virtual size_t EstRequestCpuTime(
            labstor::ipc::request *request){ return 1; };
    virtual size_t EstRequestIOTime(
            labstor::ipc::request *request){ return 0; };
    virtual bool ProcessRequest(
            labstor::queue_pair *qp,
            labstor::ipc::request *request,
            labstor::credentials *creds) { return true; };
    virtual void StateUpdate(labstor::Module *prior) {}
};
typedef labstor::Module* (*create_module_fn)(void);
typedef labstor::id (*get_module_id_fn)(void);

struct ModulePath {
    std::string client;
    std::string server;
};

enum class ModulePathType {
    kClient,
    kServer
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
        AUTO_TRACE("")
        ModuleHandle module_info;
        void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if(handle == NULL) {
            throw DLSYM_MODULE_NOT_FOUND.format(path, dlerror());
        }
        labstor::create_module_fn create_module = (labstor::create_module_fn)dlsym(handle, "create_module");
        if(create_module == NULL) {
            throw DLSYM_MODULE_NO_CONSTRUCTOR.format(path);
        }
        labstor::get_module_id_fn get_module_id = (labstor::get_module_id_fn)dlsym(handle, "get_module_id");
        if(get_module_id == NULL) {
            throw DLSYM_MODULE_NO_CONSTRUCTOR.format(path);
        }
        module_id = get_module_id();
        module_info.constructor_ = create_module;
        module_info.handle_ = handle;
        return module_info;
    }

    void SetModuleConstructor(labstor::id module_id, labstor::ModuleHandle &module_info) {
        AUTO_TRACE("", (size_t)this)
        mutex_.lock();
        TRACEPOINT("Adding module", module_id.key_, std::hash<labstor::id>()(module_id))
        if(pkg_pool_.find(module_id) != pkg_pool_.end()) {
            dlclose(pkg_pool_[module_id].handle_);
        }
        TRACEPOINT("Here")
        pkg_pool_.emplace(module_id, module_info);
        mutex_.unlock();
    }

    create_module_fn GetModuleConstructor(labstor::id module_id) {
        ModuleHandle module_info;
        TRACEPOINT(module_id.key_, std::hash<labstor::id>()(module_id))
        mutex_.lock();
        if(pkg_pool_.find(module_id) == pkg_pool_.end()) {
            mutex_.unlock();
            throw labstor::INVALID_MODULE_ID.format(module_id.key_);
        }
        module_info = pkg_pool_[module_id];
        mutex_.unlock();
        return module_info.constructor_;
    }
};

}

#endif //LABSTOR_MODULE_H
