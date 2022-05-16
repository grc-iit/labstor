//
// Created by lukemartinlogan on 5/14/22.
//

#include <labstor/userspace/types/labstack.h>

int main(int argc, char **argv) {
    labstor::LabStack stack;
    for(int i = 1; i < argc; ++i) {
        stack.UnmountLabStack(argv[1]);
    }
}