//
// Created by lukemartinlogan on 9/12/21.
//

#ifndef LABSTOR_BASICS_H
#define LABSTOR_BASICS_H

#include <cstring>
#include <unordered_map>

namespace labstor {

struct labstor_id {
    char id[256];
    bool operator==(const labstor_id &other) const {
        return strncmp(id, other.id, 256);
    }
};

struct credentials {
    int pid;
    int uid;
    int gid;
    int priority;
};

struct setup_request {
    int num_queues;
    int queue_size;
    void *starting_address;
};

}

template <>
struct std::hash<labstor::labstor_id> {
    std::size_t operator()(const labstor::labstor_id& key) const {
        return std::hash<char*>{}((char*)key.id);
    }
};

#endif //LABSTOR_BASICS_H
