//
// Created by lukemartinlogan on 12/9/21.
//

#include <unistd.h>
#include <stdio.h>

int main() {
    printf("PID: %d\n", getpid());
    printf("TID: %d\n", gettid());
    while(true) {
        sleep(1);
    }
}