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

namespace labstor {

struct module {
    labstor::id module_id;
    uint32_t runtime_id;
    void (*process_request_fn)(struct queue_pair *qp, void *request, struct credentials *creds);
    void* (*get_ops)(void);
};

struct module_paths {
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
    std::unordered_map<labstor::id, labstor::module_paths> paths_;
    std::unordered_map<labstor::id, uint32_t> runtime_ids_;
    std::unordered_map<uint32_t, labstor::module *> pkg_pool_;
    std::atomic_uint32_t cur_runtime_id;
public:
    ModuleManager() : cur_runtime_id(0) {}

    void UpdateModule(std::string path) {
        uint32_t runtime_id;
        void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        labstor::module *new_pkg = (labstor::module*)dlsym(handle, "module_");
        if(new_pkg == NULL) {
            printf("Module %s does not exist\n", path.c_str());
            return;
        }
        labstor::module *old_pkg;

        mutex_.lock();
        if(runtime_ids_.find(new_pkg->module_id) != runtime_ids_.end()) {
            runtime_id = runtime_ids_[new_pkg->module_id];
            old_pkg = pkg_pool_[runtime_id];
            //Pause all future ops using this module, wait for all current ops to complete.
            //if(old_pkg.pause) { old_pkg.pause(); }
            pkg_pool_[runtime_id] = new_pkg;
            //if(old_pkg.resume) { old_pkg.resume(); }
        }
        else {
            runtime_id = cur_runtime_id++;
            runtime_ids_[new_pkg->module_id] = runtime_id;
            pkg_pool_[runtime_id] = new_pkg;
        }
        mutex_.unlock();
    }

    labstor::module *GetModule(labstor::id module_id) {
        try {
            int runtime_id = runtime_ids_[module_id];
            return pkg_pool_[runtime_id];
        }
        catch(...) {
            return nullptr;
        }
    }

    labstor::module *GetModule(int runtime_id) {
        try {
            return pkg_pool_[runtime_id];
        }
        catch(...) {
            return nullptr;
        }
    }

    void AddModulePaths(labstor::id module_id, labstor::module_paths paths) {
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
