//
// Created by lukemartinlogan on 5/30/21.
//

#ifndef LABSTOR_ERRORS_H
#define LABSTOR_ERRORS_H

#include <labstor/util/error.h>

namespace labstor {
    const Error FILE_NOT_FOUND(0, "File not found at {}");
    const Error INVALID_STORAGE_TYPE(1, "{} is not a valid storage method");
    const Error INVALID_SERIALIZER_TYPE(2, "{} is not a valid serializer type");
    const Error INVALID_TRANSPORT_TYPE(3, "{} is not a valid transport type");

    const Error DLSYM_MODULE_NOT_FOUND(100, "Module {} was not found");
    const Error DLSYM_MODULE_NO_CONSTRUCTOR(101, "Module {} has no constructor");

    const Error UNIX_SOCKET_FAILED(200, "Failed to create socket: {}");
    const Error UNIX_BIND_FAILED(201, "Failed to bind socket: {}");
    const Error UNIX_CONNECT_FAILED(202, "Failed to connect over socket: {}");
    const Error UNIX_SENDMSG_FAILED(203, "Failed to send message over socket: {}");
    const Error UNIX_RECVMSG_FAILED(204, "Failed to receive message over socket: {}");

    const Error INVALID_RING_BUFFER_SIZE(300, "Failed to allocate ring buffer, {} is too small to support {} requests.");
}


#endif //LABSTOR_ERRORS_H
