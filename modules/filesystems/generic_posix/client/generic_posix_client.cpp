//
// Created by lukemartinlogan on 8/20/21.
//

#include <registrar/client/registrar_client.h>
#include <labstor/userspace/client/ipc_manager.h>
#include "generic_posix_client.h"
#include "lib/posix_client.h"
#include <mutex>

void labstor::GenericPosix::Client::Initialize() {
    AUTO_TRACE("")
    //Retreive the namespace ID from the server
    if(lock_.try_lock()) {
        if(!is_initialized_) {
            ipc_manager_->Connect();
            labstor::Registrar::Client registrar;
            ns_id_ = registrar.GetNamespaceID(GENERIC_POSIX_MODULE_ID);
            TRACEPOINT("Found generic posix namespace ID", ns_id_)
            is_initialized_ = true;
            fd_min_ = LABSTOR_FD_MIN; //TODO: Should be queried from the server
            prefix_ = LABSTOR_PATH_PREFIX;
        }
    }
}

int labstor::GenericPosix::Client::Open(const char *path, int oflag) {
    AUTO_TRACE(path)
    int fd;

    //Determine if the path is a labstor path
    if(strncmp(path, prefix_.c_str(), prefix_.size()) != 0) {
        return LABSTOR_GENERIC_FS_PATH_NOT_FOUND;
    }

    //Determine the module belonging to the path
    int len = strlen(path);
    uint32_t ns_id;
    labstor::Posix::Client *module;
    while(len > 0) {
        labstor::ipc::string path_str(std::string(path, len));
        TRACEPOINT(std::string(path, len))
        if(namespace_->GetIfExists(path_str, ns_id)) {
            TRACEPOINT("FOUND IT!")
            module = namespace_->Get<labstor::Posix::Client>(ns_id);
            if(module == nullptr) {
                module = namespace_->LoadClientModule<labstor::Posix::Client>(ns_id);
            }
            break;
        }
        len = PriorSlash(path, len);
    }
    if(len == 0) {
        return LABSTOR_GENERIC_FS_PATH_NOT_FOUND;
    }

    //Allocate an fd & track which module the fd belongs to
    fd = AllocateFD();
    fd_to_ns_id_.emplace(fd, ns_id);
    TRACEPOINT("FD", fd)

    //Call the client's implementation of open()
    fd = module->Open(fd, path, len, oflag);
    return fd;
}

int labstor::GenericPosix::Client::Close(int fd) {
    if(fd < fd_min_) { return LABSTOR_INVALID_FD; }
    int ns_id = fd_to_ns_id_[fd];
    labstor::Posix::Client *client = namespace_->Get<labstor::Posix::Client>(ns_id);
    client->Close(fd);
    fd_to_ns_id_.erase(fd);
    FreeFD(fd);
    return 0;
}

labstor::ipc::qtok_t labstor::GenericPosix::Client::AIO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size) {
    AUTO_TRACE("")
    if(fd < fd_min_) { return labstor::ipc::qtok_t(); }
    uint32_t ns_id = fd_to_ns_id_[fd];
    labstor::Posix::Client *client = namespace_->Get<labstor::Posix::Client>(ns_id);
    return client->AIO(op, fd, buf, size);
}

ssize_t labstor::GenericPosix::Client::IO(labstor::GenericPosix::Ops op, int fd, void *buf, size_t size) {
    AUTO_TRACE("")
    if(fd < fd_min_) { return -1; }
    uint32_t ns_id = fd_to_ns_id_[fd];
    labstor::Posix::Client *client = namespace_->Get<labstor::Posix::Client>(ns_id);
    return client->IO(op, fd, buf, size);
}

LABSTOR_MODULE_CONSTRUCT(labstor::GenericPosix::Client, GENERIC_POSIX_MODULE_ID);