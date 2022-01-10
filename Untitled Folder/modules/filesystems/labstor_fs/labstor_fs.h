//
// Created by lukemartinlogan on 12/30/21.
//

#ifndef LABSTOR_LABSTOR_FS_H
#define LABSTOR_LABSTOR_FS_H

#include <cstring>
#include <labstor/types/data_structures/shmem_request.h>
#include <generic_posix/generic_posix.h>

#define LABFS_MODULE_ID "LabFS"

namespace labstor::LabFS {

struct init_request : public labstor_request {
    char iosched_mount_[];
    void Start(int ns_id, char *iosched_mount) {
        SetNamespaceID(ns_id);
        SetOp(static_cast<int>(labstor::GenericPosix::Ops::kInit));
        strcpy(iosched_mount_, iosched_mount);
    }
    void Complete(int code) {
        SetCode(code);
    }
    char* GetIOsched() {
        return iosched_mount_;
    }
};

}

#endif //LABSTOR_LABSTOR_FS_H
