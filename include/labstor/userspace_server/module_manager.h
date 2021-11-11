//
// Created by lukemartinlogan on 11/4/21.
//

#ifndef LABSTOR_SERVER_MODULE_MANAGER_H
#define LABSTOR_SERVER_MODULE_MANAGER_H

#include <labstor/types/module.h>

namespace labstor::Server {

class ModuleManager : public ModuleTable {
private:
    std::atomic_uint32_t cur_runtime_id;
    std::unordered_map<labstor::id, labstor::ModulePath> paths_;
public:
    void UpdateModule(std::string path);
    void AddModulePaths(labstor::id module_id, labstor::ModulePath paths);
    std::string GetModulePath(labstor::id module_id, ModulePathType type);
};

}

#endif //LABSTOR_MODULE_MANAGER_H
