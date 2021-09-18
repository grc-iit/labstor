//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_MODULE_H
#define LABSTOR_MODULE_H

#include <dlfcn.h>
#include <unordered_map>
#include <atomic>

#include <labstor/types/basics.h>

namespace labstor {

struct module {
    labstor::id module_id;
    uint32_t runtime_id;
    void (*process_request_fn)(struct queue_pair *qp, void *request, struct credentials *creds);
    void* (*get_ops)(void);
};

class ModulePool {
private:
    std::unordered_map<labstor::id, uint32_t> runtime_ids_;
    std::unordered_map<uint32_t, labstor::module *> pkg_pool_;
    std::atomic_uint32_t cur_runtime_id;
public:
    ModulePool() : cur_runtime_id(0) {}

    void UpdateModule(std::string path) {
        uint32_t runtime_id;
        void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        labstor::module *new_pkg = (labstor::module*)dlsym(handle, "module_");
        labstor::module *old_pkg;

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
};

}

#endif //LABSTOR_MODULE_H
