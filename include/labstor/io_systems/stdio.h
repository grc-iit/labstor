//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_POSIX_H
#define LABSTOR_POSIX_H

#include <unistd.h>

struct stdio_request {
    int op;
    union {
        struct posix_open_request open;
        struct posix_io_request io;
    } rq;
};

struct stdio_open_request {
    char *path;
    int flags;
    mode_t mode;
};

struct stdio_io_request {
    int fd;
    void *buf;
    ssize_t count;
};

struct stdio_ops {
    int (*fopen)(const char *pathname, int flags, mode_t mode);
    ssize_t (*fread)(int fd, void *buf, ssize_t count);
    ssize_t (*fwrite)(int fd, void *buf, ssize_t count);
    off_t (*fseek)(int fd, off_t offset, int whence);
    int (*fclose)(int fd);
};

#endif //LABSTOR_POSIX_H
