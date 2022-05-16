//
// Created by lukemartinlogan on 5/14/22.
//

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
