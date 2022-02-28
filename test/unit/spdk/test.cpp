//
// Created by lukemartinlogan on 2/27/22.
//

#include <modules/iosched/spdk/lib/spdk_lib.h>

void help() {
    printf("USAGE: ./test [opt (1/2)] [trid (case 2)] [ns_id (case 2)] [num_qps (case 2)]\n");
    exit(1);
}

int main(int argc, char **argv) {
    labstor::SPDK::Context lib;
    char *trid = nullptr;
    int ns_id = 0, num_qps = 0;
    int opt = 0;
    if(argc == 1) {
        help();
    }
    opt = atoi(argv[1]);
    if(opt == 2 && argc != 5) {
        help();
    }
    if(opt == 2) {
        trid = argv[2];
        ns_id = atoi(argv[3]);
        num_qps = atoi(argv[4]);
    }

    printf("OPT: %d\n", opt);

    switch(opt) {
        //List NVME devices
        case 1: {
            lib.Init();
            lib.Probe(true);
            break;
        }

        //Allocate queue pairs
        case 2: {
            lib.Init();
            lib.Probe(false);
            if(!lib.SelectDevice(trid, ns_id)) {
                printf("TRID: %s\n", trid);
                printf("NSID: %d\n", ns_id);
                printf("Failed to find device\n");
                exit(1);
            }
            lib.CreateQueuePairs(num_qps);
            break;
        }
        default: {
            help();
        }
    }
}