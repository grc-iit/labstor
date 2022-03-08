//
// Created by lukemartinlogan on 2/25/22.
//

#include <spdk/spdk.h>

int main(int argc, char **argv) {
    labstor::SPDK::Context lib;
    lib.Init();
    lib.Probe(true);
}

