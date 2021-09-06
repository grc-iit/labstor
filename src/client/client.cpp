//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <linux/sched.h>    /* Definition of struct clone_args */
#include <sched.h>          /* Definition of CLONE_* constants */
#include <sys/syscall.h>    /* Definition of SYS_* constants */
#include <unistd.h>
#include <pthread.h>
#include <memory>

#include <labstor/util/singleton.h>
#include <ptrhead>
#include <sys/socket.h>

#include <unordered_map>
#include <list>

#define TRUSTED_SERVER_PATH "/tmp/server"

void connect_to_server(void) {
    int id_;
    int sockfd_;
    pthread_t accept_thread_;
    pthread_t loop_thread_;
    pthread_attr_t attr_ = 1;

    int ret;
    struct sockaddr_un serveraddr;

    id_ = id;
    sockfd_ = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(sockfd_ < 0) {
        perror("socket() failed");
        return;
    }

    ret = setsockopt(sockfd_, SOL_SOCKET, SO_PASSCRED, (void*)&attr_, sizeof(attr_));
    if(ret < 0) {
        perror("setsockopt() failed");
        return;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, TRUSTED_SERVER_PATH);
    ret = bind(sockfd_, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
    if(ret < 0) {
        perror("bind() failed");
        return;
    }

    ret = listen(sockfd_, 1024);
    if(ret < 0) {
        perror("listen() failed");
        return;
    }

    pthread_create(&accept_thread_, );
}
