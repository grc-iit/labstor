
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

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
    printf("HERE1\n");
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