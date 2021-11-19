//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_SERVER_NAMESPACE_H
#define LABSTOR_SERVER_NAMESPACE_H

#include <labstor/types/basics.h>
#include <labstor/ipc/unordered_map.h>

namespace labstor::Server {

struct Namespace {
    int region_id;
    labstor::unordered_map<labstor::id, labstor::Module*> namespace_;
};

}

#endif //LABSTOR_SERVER_NAMESPACE_H
