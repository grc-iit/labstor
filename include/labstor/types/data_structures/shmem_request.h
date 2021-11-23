//
// Created by lukemartinlogan on 11/22/21.
//

#ifndef LABSTOR_REQUEST_H
#define LABSTOR_REQUEST_H

#ifdef __cplusplus

namespace labstor::ipc {

struct request {
    uint32_t qtok_;
    uint32_t ns_id_;
    inline uint32_t GetNamespaceEntry() { return ns_id_; }
};

}

#endif

#endif //LABSTOR_REQUEST_H
