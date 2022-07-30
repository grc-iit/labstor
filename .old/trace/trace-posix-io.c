
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/mman.h>
#include <linux/mman.h>
#include <errno.h>

void buffered_write(size_t blocksz, int count, char *path)
{
    char *buffer = (char*)aligned_alloc(4096, blocksz);
    if(buffer == NULL) {
        printf("Could not allocate memory (buffered_write)\n");
        exit(1);
    }
    memset(buffer, 0, blocksz);

    FILE *f = fopen(path, "w");
    if(f == NULL) {
        printf("Could not open file (buffered_write) (%s)\n", path);
        exit(1);
    }
    for(int i = 0; i < count; i++) {
        int cnt = fwrite(buffer, 1, blocksz, f);
        if (cnt != blocksz) {
            printf("Could not write to file (buffered_write) (%s)\n", path);
            exit(1);
        }
    }
    fclose(f);
    free(buffer);
}

void direct_write(size_t blocksz, int count, char *path)
{
    char *buffer = (char*)aligned_alloc(4096, blocksz);
    if(buffer == NULL) {
        printf("Could not allocate memory (direct_write)\n");
        exit(1);
    }
    memset(buffer, 0, blocksz);

    int fd = open(path, O_DIRECT | O_WRONLY | O_CREAT);
    if(fd < 0) {
        printf("Could not open file (direct_write) (%s)\n", path);
        perror("");
        exit(1);
    }
    for(int i = 0; i < count; i++) {
        int cnt = write(fd, buffer, blocksz);
        if (cnt < 0) {
            printf("Could not write to file (direct_write) (%s)\n", path);
            perror("");
            exit(1);
        }
    }
    close(fd);
    free(buffer);
}

void buffered_read(size_t blocksz, int count, char *path)
{
    char *buffer = (char*)aligned_alloc(4096, blocksz);
    if(buffer == NULL) {
        printf("Could not allocate memory\n");
        exit(1);
    }
    memset(buffer, 0, blocksz);

    FILE *f = fopen(path, "r");
    if(f == NULL) {
        printf("Could not open file (buffered_read) (%s)\n", path);
        exit(1);
    }
    for(int i = 0; i < count; i++) {
        int cnt = fread(buffer, 1, blocksz, f);
        if (cnt != blocksz) {
            printf("Could not write to file (%s)\n", path);
            exit(1);
        }
    }
    fclose(f);
    free(buffer);
}

void direct_read(size_t blocksz, int count, char *path)
{
    char *buffer = (char*)aligned_alloc(4096, blocksz);
    if(buffer == NULL) {
        printf("Could not allocate memory (direct_read)\n");
        exit(1);
    }
    memset(buffer, 0, blocksz);

    int fd = open(path, O_DIRECT | O_RDONLY);
    if(fd < 0) {
        printf("Could not open file (direct_read) (%s)\n", path);
        exit(1);
    }
    for(int i = 0; i < count; i++) {
        int cnt = read(fd, buffer, blocksz);
        if (cnt < 0) {
            printf("Could not write to file (direct_read) (%s)\n", path);
            exit(1);
        }
    }
    close(fd);
    free(buffer);
}

size_t to_size(char *num)
{
    int len = strlen(num);
    if(num[len-1] == 'k' || num[len-1] == 'K') {
        return (size_t)atoi(num)*(1ull<<10);
    }
    else if(num[len-1] == 'm' || num[len-1] == 'M') {
        return (size_t)atoi(num)*(1ull<<20);
    }
    else if(num[len-1] == 'g' || num[len-1] == 'G') {
        return (size_t)atoi(num)*(1ull<<30);
    }
    else if(num[len-1] == 'b' || num[len-1] == 'B') {
        return (size_t)atoi(num);
    }

    printf("Invalid integer type\n");
    exit(1);
}

int main(int argc, char **argv)
{
    if(argc != 5) {
        printf("Usage: /path/to/time-transfer-methods [test-case] [block-size [kmg]] [count] [file-path]\n");
        printf("0: buffered_write\n");
        printf("1: direct_write\n");
        printf("2: buffered_read\n");
        printf("3: direct_read\n");
        exit(1);
    }

    int test = atoi(argv[1]);
    size_t blocksz = to_size(argv[2]);
    int count = atoi(argv[3]);
    char *path = argv[4];

    printf("SLEEPING BRIEFLY!\n");
    sleep(5);
    printf("STARTING!\n");

    switch(test) {
        case 0: {
            buffered_write(blocksz, count, path);
            break;
        }
        case 1: {
            direct_write(blocksz, count, path);
            break;
        }
        case 2: {
            buffered_read(blocksz, count, path);
            break;
        }
        case 3: {
            direct_read(blocksz, count, path);
            break;
        }
    }
}