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
#include <labstor/ipc/request_queue.h>
#include <labstor/util/errors.h>

#ifndef __CLION_IDE__
#define LABSTOR_MODULE_CONSTRUCT(MODULE_NAME) \
    extern "C" { labstor::Module* create_module() { return new MODULE_NAME(); } }
#endif

namespace labstor {

class Module {
protected:
    labstor::id module_id_;
    uint32_t runtime_id_;
public:
    inline labstor::id GetModuleID() { return module_id_; }
    inline uint32_t GetRuntimeID() { return runtime_id_; }
    inline void SetRuntimeID(uint32_t runtime_id) { runtime_id_ = runtime_id; }

    virtual void ProcessRequest(labstor::ipc::queue_pair *qp, void *request, labstor::credentials *creds) = 0;
};
typedef labstor::Module* (*create_module_fn)(void);

struct ModulePath {
    std::string client;
    std::string trusted_client;
    std::string server;
};

enum class ModulePathType {
    CLIENT,
    TRUSTED_CLIENT,
    SERVER
};

class ModuleManager {
private:
    std::mutex mutex_;
    std::unordered_map<labstor::id, labstor::ModulePath> paths_;
    std::unordered_map<labstor::id, uint32_t> runtime_ids_;
    std::unordered_map<uint32_t, labstor::Module *> pkg_pool_;
    std::atomic_uint32_t cur_runtime_id;
public:
    ModuleManager() : cur_runtime_id(0) {}

    void UpdateModule(std::string path) {
        uint32_t runtime_id;
        void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if(handle == NULL) {
            throw DLSYM_MODULE_NOT_FOUND.format(path);
        }
        labstor::create_module_fn create_module = (labstor::create_module_fn)dlsym(handle, "create_module");
        if(create_module == NULL) {
            throw DLSYM_MODULE_NO_CONSTRUCTOR.format(path);
        }
        labstor::Module *new_pkg = create_module();
        labstor::Module *old_pkg;

        mutex_.lock();
        if(runtime_ids_.find(new_pkg->GetModuleID()) != runtime_ids_.end()) {
            runtime_id = runtime_ids_[new_pkg->GetModuleID()];
            old_pkg = pkg_pool_[runtime_id];
            pkg_pool_[runtime_id] = new_pkg;
        }
        else {
            runtime_id = cur_runtime_id++;
            runtime_ids_[new_pkg->GetModuleID()] = runtime_id;
            pkg_pool_[runtime_id] = new_pkg;
        }
        mutex_.unlock();
    }

    labstor::Module *GetModule(labstor::id module_id) {
        try {
            int runtime_id = runtime_ids_[module_id];
            return pkg_pool_[runtime_id];
        }
        catch(...) {
            return nullptr;
        }
    }

    labstor::Module *GetModule(int runtime_id) {
        try {
            return pkg_pool_[runtime_id];
        }
        catch(...) {
            return nullptr;
        }
    }

    void AddModulePaths(labstor::id module_id, labstor::ModulePath paths) {
        mutex_.lock();
        paths_[module_id] = paths;
        mutex_.unlock();
    }

    std::string GetModulePath(labstor::id module_id, ModulePathType type) {
        switch(type) {
            case ModulePathType::CLIENT: {
                return paths_[module_id].client;
            }
            case ModulePathType::TRUSTED_CLIENT: {
                return paths_[module_id].trusted_client;
            }
            case ModulePathType::SERVER: {
                return paths_[module_id].server;
            }
        }
    }
};

}

#endif //LABSTOR_MODULE_H
