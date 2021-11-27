//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_SERVER_H
#define LABSTOR_SERVER_H

#include <pthread.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace labstor::Server {

struct ConfigurationManager {
    YAML::Node config_;
    void LoadConfig(char *path) {
        config_ = YAML::LoadFile(path);
    }
};

}

#endif //LABSTOR_SERVER_H
