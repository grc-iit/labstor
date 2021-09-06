#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define CHUNK_SIZE (1<<24)
#define PAGE_SIZE (1<<12)

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
    else if('0' <= num[len-1] && num[len-1] <= '9') {
        return (size_t)atoi(num);
    }

    printf("Invalid integer type: %s (%c)\n", num, num[len-1]);
    exit(1);
}

int main(int argc, char **argv)
{
    if(argc != 5) {
        printf("./verify-write [path] [off] [size] [val]\n");
        exit(1);
    }
    char *path = argv[1];
    size_t off = to_size(argv[2]);
    size_t size = to_size(argv[3]);
    int val = atoi(argv[4]);

    /*if((off % PAGE_SIZE) || (size % PAGE_SIZE)) {
        printf("Offset or size is not a multiple of PAGE_SIZE");
        exit(1);
    }*/

    //sleep(8);
    printf("STARTING!\n");

    int fd = open(path, O_RDWR | O_DIRECT);
    if(fd < 0) {
        printf("Could not open: %s\n", argv[1]);
        exit(1);
    }

    char *buf = aligned_alloc(4096, CHUNK_SIZE);
    for(size_t i = 0; i < size; i += CHUNK_SIZE) {
        int chunk_size = (i+1)*CHUNK_SIZE <= size ? CHUNK_SIZE : size%CHUNK_SIZE;
        printf("PREADING!\n");
        pread(fd, buf, chunk_size, i+off);
        for(int j = 0; j < chunk_size; ++j) {
            if(buf[j] != val) {
                printf("INVALID: %x != %x at byte %lu\n", buf[j], val, i+j);
                goto done;
            }
        }
    }
    printf("SUCCESS!\n");

    done:
    printf("%x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
    close(fd);
    free(buf);
}