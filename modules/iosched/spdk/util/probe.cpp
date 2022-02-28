//
// Created by lukemartinlogan on 2/25/22.
//

#include <spdk/lib/spdk_lib.h>

int main(int argc, char **argv) {
    labstor::SPDK::Context lib;
    lib.Init();
    lib.Probe(true);
}

