//
// Created by lukemartinlogan on 8/21/21.
//

#ifndef LABSTOR_UNORDERED_MAP_H
#define LABSTOR_UNORDERED_MAP_H

namespace labstor::ipc {

template<typename T>
struct bucket {}

template<typename S, typename T>
class unordered_map {
private:
    struct obj_allocator *alloc_;
    struct bucket *buckets;
public:
    void init(std::string id) {

    }
}
};

}

#endif //LABSTOR_UNORDERED_MAP_H
