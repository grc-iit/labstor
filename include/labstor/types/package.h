//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_PACKAGE_H
#define LABSTOR_PACKAGE_H

#include <dlfcn.h>
#include <unordered_map>
#include <atomic>

#include <labstor/types/basics.h>

namespace labstor {

struct package {
    labstor::labstor_id pkg_id_;
    uint32_t runtime_id_;
    void (*process_request_fn)(struct queue_pair *qp, void *request, struct credentials *creds);
    void* (*get_ops)(void);
    int req_size;
};

class PackagePool {
private:
    std::unordered_map<labstor::labstor_id, uint32_t> runtime_ids_;
    std::unordered_map<uint32_t, labstor::package *> pkg_pool_;
    std::atomic_uint32_t cur_runtime_id_;
public:
    PackagePool() : cur_runtime_id_(0) {}

    void UpdatePackage(std::string path) {
        uint32_t runtime_id;
        void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        labstor::package *new_pkg = (labstor::package*)dlsym(handle, "package_");
        labstor::package *old_pkg;

        if(runtime_ids_.find(new_pkg->pkg_id_) != runtime_ids_.end()) {
            runtime_id = runtime_ids_[new_pkg->pkg_id_];
            old_pkg = pkg_pool_[runtime_id];
            //Pause all future ops using this package, wait for all current ops to complete.
            //if(old_pkg.pause) { old_pkg.pause(); }
            pkg_pool_[runtime_id] = new_pkg;
            //if(old_pkg.resume) { old_pkg.resume(); }
        }
        else {
            runtime_id = cur_runtime_id_++;
            runtime_ids_[new_pkg->pkg_id_] = runtime_id;
            pkg_pool_[runtime_id] = new_pkg;
        }
    }

    labstor::package *GetPackage(labstor::labstor_id pkg_id) {
        try {
            int runtime_id = runtime_ids_[pkg_id];
            return pkg_pool_[runtime_id];
        }
        catch(...) {
            return nullptr;
        }
    }

    labstor::package *GetPackage(int runtime_id) {
        try {
            return pkg_pool_[runtime_id];
        }
        catch(...) {
            return nullptr;
        }
    }
};

}

#endif //LABSTOR_PACKAGE_H
