//
// Created by lukemartinlogan on 11/4/21.
//

#ifndef LABSTOR_CLIENT_MODULE_MANAGER_H
#define LABSTOR_CLIENT_MODULE_MANAGER_H

#include <labstor/userspace/types/module.h>

namespace labstor::Client {

class ModuleManager : public ModuleTable {
public:
    labstor::id UpdateModule(std::string module_path);
};

}

#endif //LABSTOR_CLIENT_MODULE_MANAGER_H
