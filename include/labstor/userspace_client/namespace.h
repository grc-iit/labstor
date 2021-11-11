//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_CLIENT_NAMESPACE_H
#define LABSTOR_CLIENT_NAMESPACE_H

#include <labstor/ipc/unordered_map.h>

namespace labstor::Client {

struct Namespace {
    int region_id;
    labstor::ipc::unordered_map<std::string, labstor::Module*> namespace_;
};

}

#endif //LABSTOR_CLIENT_NAMESPACE_H
