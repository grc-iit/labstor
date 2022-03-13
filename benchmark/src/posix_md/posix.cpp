//
// Created by lukemartinlogan on 3/10/22.
//

#include <unistd.h>
#include <fcntl.h>
#include <labstor/userspace/util/timer.h>

int main() {
    labstor::HighResMonotonicTimer t;
    int count = 256;
    t.Resume();
    for(int i = 0; i < count; ++i) {
        int fd = open("/dev/nvme0n1", O_RDWR);
        //close(fd);
    }
    t.Pause();

    printf("MOps: %f\n", count/t.GetUsec());
}