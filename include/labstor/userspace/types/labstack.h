
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

#ifndef LABSTOR_LABSTACK_H
#define LABSTOR_LABSTACK_H

#define LABSTOR_CLIENT
#include <yaml-cpp/yaml.h>
#include "labmods/registrar/client/registrar_client.h"
#include "labstor/userspace/client/namespace.h"

namespace labstor {

class LabStack {
public:
    void MountLabStack(char *path) {
        YAML::Node config = YAML::LoadFile(path);
        std::string labstack_id = config["mount_point"].as<std::string>();
        std::list<YAML::Node> dag;
        for(auto vertex : config["dag"]) {
            dag.emplace_front(vertex);
        }
        for(auto vertex : dag) {
            std::string labmod_uuid = vertex["labmod_uuid"].as<std::string>();
            labstor::Module *module = LABSTOR_NAMESPACE->LoadClientModule<labstor::Module>(labmod_uuid);
            if(module == nullptr) {
                module->Register(vertex);
            }
        }
        LABSTOR_REGISTRAR->MountLabStack(labstack_id, path);
    }
    void UnmountLabStack(char *path) {
        LABSTOR_REGISTRAR->PushUpgrade(path);
    }
    void ModifyLabStack(char *path) {
        LABSTOR_REGISTRAR->PushUpgrade(path);
    }
};

}

#endif //LABSTOR_LABSTACK_H