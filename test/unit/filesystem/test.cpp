//
// Created by lukemartinlogan on 1/19/22.
//

#include <sys/types.h>
#include <sys/stat.h>
#include <aio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <cstdio>
#include <errno.h>
#include <vector>
#include "labstor/userspace/util/error.h"

int main(int argc, char **argv) {
    char *path = argv[1];
    printf("PATH: %s\n", path);
    //Open file
    LABSTOR_ERROR_HANDLE_START()
    int fd = open(path, O_DIRECT | O_CREAT | O_RDWR, 0x644);
    printf("FD: %d\n", fd);
    if(fd < 0) {
        perror("Could not open/create file\n");
        exit(1);
    }

    printf("fd: %d\n", fd);
    LABSTOR_ERROR_HANDLE_END()

    //close(fd);
}