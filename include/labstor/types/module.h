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

    virtual void ReinforceRequestStats(void *request, size_t time_measure) {};
    virtual size_t EstRequestTime(void *request){ return 1; };
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
    SERVER
};

class ModuleTable {
private:
    std::mutex mutex_;
    std::unordered_map<labstor::id, uint32_t> runtime_ids_;
    std::unordered_map<uint32_t, labstor::Module *> pkg_pool_;
public:
    ModuleTable() = default;

    labstor::Module* OpenModule(std::string path) {
        void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if(handle == NULL) {
            throw DLSYM_MODULE_NOT_FOUND.format(path);
        }
        labstor::create_module_fn create_module = (labstor::create_module_fn)dlsym(handle, "create_module");
        if(create_module == NULL) {
            throw DLSYM_MODULE_NO_CONSTRUCTOR.format(path);
        }
        return create_module();
    }

    void SetModule(labstor::Module *mod, uint32_t runtime_id) {
        mutex_.lock();
        mod->SetRuntimeID(runtime_id);
        runtime_ids_[mod->GetModuleID()] = runtime_id;
        pkg_pool_[runtime_id] = mod;
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

    uint32_t GetRuntimeID(labstor::id module_id) {
        labstor::Module* mod = GetModule(module_id);
        if(mod) {
            return mod->GetRuntimeID();
        }
        return -1;
    }

    labstor::Module *GetModule(int runtime_id) {
        try {
            return pkg_pool_[runtime_id];
        }
        catch(...) {
            return nullptr;
        }
    }
};

}

#endif //LABSTOR_MODULE_H
