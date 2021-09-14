//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <memory>

#include <labstor/userspace_client/client.h>
#include <labstor/util/singleton.h>
#include <sys/socket.h>

#include <unordered_map>
#include <list>

#define TRUSTED_SERVER_PATH "/tmp/labstor_trusted_server"

bool labstor::LabStorClientContext::Connect(int num_queues, size_t queue_size) {
    struct msghdr msgh;
    struct iovec iov;
    int server_fd, opt;
    ssize_t ret;
    struct sockaddr_un client_addr;
    struct sockaddr_un server_addr;

    //Create UDP socket
    memset(&client_addr, 0x0, sizeof(struct sockaddr_un));
    client_addr.sun_family = AF_UNIX;
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printf("Error: socket failed [%s]\n", strerror(errno));
        return false;
    }
    if (bind(server_fd, (struct sockaddr *) &client_addr, sizeof(struct sockaddr_un)) == -1) {
        printf("Error: bind failed [%s]\n", strerror(errno));
        close(server_fd);
        return false;
    }

    //Set server address
    memset(&server_addr, 0x0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, TRUSTED_SERVER_PATH, sizeof(server_addr.sun_path)-1);
    if (connect(server_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_un)) == -1) {
        printf("Error: connect failed [%s]\n", strerror(errno));
        close(server_fd);
        return false;
    }

    //Setup IPC request
    labstor::setup_request request;
    request.num_queues = num_queues;
    request.queue_size = queue_size;
    request.starting_address = malloc(num_queues * queue_size);

    //Send an IPC request to the server
    ret = send(server_fd, (void*)&request, sizeof(labstor::setup_request), 0);
    if (ret == -1) {
        printf("Error: sendmsg failed [%s]\n", strerror(errno));
        return false;
    }

    //Receive SHMEM IPCs
    labstor::setup_request reply;
    ret = recv(server_fd, &reply, sizeof(labstor::setup_request), 0);
    if (ret == -1) {
        printf("Error: recvmsg failed [%s]\n", strerror(errno));
        return false;
    }

    return true;
}
