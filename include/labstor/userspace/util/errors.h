//
// Created by lukemartinlogan on 5/30/21.
//

#ifndef LABSTOR_ERRORS_H
#define LABSTOR_ERRORS_H

#include <labstor/userspace/util/error.h>

namespace labstor {
    const Error FILE_NOT_FOUND(0, "File not found at {}");
    const Error INVALID_STORAGE_TYPE(1, "{} is not a valid storage method");
    const Error INVALID_SERIALIZER_TYPE(2, "{} is not a valid serializer type");
    const Error INVALID_TRANSPORT_TYPE(3, "{} is not a valid transport type");
    const Error INVALID_AFFINITY(3, "Could not set CPU affinity of thread: {}");
    const Error MMAP_FAILED(4, "Could not mmap file: {}");

    const Error DLSYM_MODULE_NOT_FOUND(100, "Module {} was not found");
    const Error DLSYM_MODULE_NO_CONSTRUCTOR(101, "Module {} has no constructor");

    const Error UNIX_SOCKET_FAILED(200, "Failed to create socket: {}");
    const Error UNIX_BIND_FAILED(201, "Failed to bind socket: {}");
    const Error UNIX_CONNECT_FAILED(202, "Failed to connect over socket: {}");
    const Error UNIX_SENDMSG_FAILED(203, "Failed to send message over socket: {}");
    const Error UNIX_RECVMSG_FAILED(204, "Failed to receive message over socket: {}");
    const Error UNIX_SETSOCKOPT_FAILED(205, "Failed to set socket options: {}");
    const Error UNIX_GETSOCKOPT_FAILED(205, "Failed to acquire user credentials: {}");
    const Error UNIX_LISTEN_FAILED(206, "Failed to listen for connections: {}");
    const Error UNIX_ACCEPT_FAILED(207, "Failed accept connections: {}");

    const Error INVALID_RING_BUFFER_SIZE(300, "Failed to allocate ring buffer, {} is too small to support {} requests.");
    const Error INVALID_UNORDERED_MAP_KEY(301, "No such key in map");
    const Error INVALID_QP_QUERY(302, "There is no such queue that satisifies the given properties.");

    const Error SHMEM_CREATE_FAILED(400, "Failed to allocate SHMEM");

    const Error INVALID_MODULE_ID(500, "Failed to find module {}");
    const Error INVALID_NAMESPACE_ENTRY(501, "Failed to find namespace entry");

    const Error NOT_YET_IMPLEMENTED(1000, "{} is not yet implemented");
}


#endif //LABSTOR_ERRORS_H
