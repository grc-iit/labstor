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
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/util/errors.h>

#ifndef __CLION_IDE__
#define LABSTOR_MODULE_CONSTRUCT(MODULE_NAME) \
    extern "C" {                              \
        labstor::Module* create_module() { return new MODULE_NAME(); } \
    }
#endif

namespace labstor {

class Module {
protected:
    labstor::id module_id_;
    uint32_t runtime_id_;
public:
    Module(labstor::id module_id) : module_id_(module_id) {}
    inline labstor::id GetModuleID() { return module_id_; }
    inline uint32_t GetRuntimeID() { return runtime_id_; }
    inline void SetRuntimeID(uint32_t runtime_id) { runtime_id_ = runtime_id; }

    virtual void ReinforceRequestStats(labstor::ipc::request *request, size_t time_measure_ns) {};
    virtual size_t EstRequestTime(labstor::ipc::request *request){ return 1; };
    virtual void ProcessRequest(labstor::ipc::queue_pair &qp, labstor::ipc::request *request, labstor::credentials *creds) {};
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

class ModuleTable {
private:
    std::mutex mutex_;
    std::unordered_map<labstor::id, create_module_fn> pkg_pool_;
public:
    ModuleTable() = default;

    create_module_fn OpenModule(std::string path, labstor::id &module_id) {
        void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if(handle == NULL) {
            throw DLSYM_MODULE_NOT_FOUND.format(path);
        }
        labstor::create_module_fn create_module = (labstor::create_module_fn)dlsym(handle, "create_module");
        if(create_module == NULL) {
            throw DLSYM_MODULE_NO_CONSTRUCTOR.format(path);
        }
        module_id = create_module()->GetModuleID();
        return create_module;
    }

    void SetModuleConstructor(labstor::id module_id, labstor::create_module_fn create_fn) {
        mutex_.lock();
        pkg_pool_[module_id] = create_fn;
        mutex_.unlock();
    }

    create_module_fn GetModuleConstructor(labstor::id module_id) {
        create_module_fn create_fn;
        mutex_.lock();
        create_fn = pkg_pool_[module_id];
        mutex_.unlock();
        return create_fn;
    }
};

}

#endif //LABSTOR_MODULE_H
