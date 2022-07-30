
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef LABSTOR_SERVER_MODULE_MANAGER_H
#define LABSTOR_SERVER_MODULE_MANAGER_H

#include <labstor/userspace/types/module.h>
#include <labstor/userspace/server/macros.h>
#include <labstor/userspace/server/server.h>
#include <set>

namespace labstor::Server {

class ModuleManager : public ModuleTable {
private:
    LABSTOR_CONFIGURATION_MANAGER_T labstor_config_;
    std::unordered_map<labstor::id, labstor::ModulePath> paths_;
    std::set<std::string> repos_;
public:
    ModuleManager() {
        labstor_config_ = LABSTOR_CONFIGURATION_MANAGER;
    }
    bool LoadRepos();
    void CentralizedUpdateModule(YAML::Node config);
    void DecentralizedUpdateModule(YAML::Node config);
    void AddModulePaths(labstor::id module_id, labstor::ModulePath paths);
    std::string GetModulePath(labstor::id module_id, ModulePathType type);
};

}

#endif //LABSTOR_MODULE_MANAGER_H