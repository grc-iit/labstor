//
// Created by lukemartinlogan on 12/31/21.
//

#include "posix.h"
#include "labstor_mq.h"

void io_test(labstor::IOTest *test) {
    labstor::HighResMonotonicTimer t[2];
    t[0].Resume();
    test->Write();
    t[0].Pause();

    t[1].Resume();
    test->Read();
    t[1].Pause();

    printf("Write Bandwidth: %lf MBps\n", test->GetTotalIO()/t[0].GetUsec());
    printf("Write Throughput: %lf KOps\n", test->GetTotalNumOps()/t[0].GetMsec());

    printf("Read Bandwidth: %lf MBps\n", test->GetTotalIO()/t[1].GetUsec());
    printf("Read Throughput: %lf KOps\n", test->GetTotalNumOps()/t[1].GetMsec());
}

int main(int argc, char **argv) {
    if(argc != 5) {
        printf("USAGE: ./io_test [io_method] [block_size_kb] [total_size_mb] [path]");
        exit(1);
    }
    std::string io_method = std::string(argv[1]);
    uint64_t block_size = atoi(argv[2])*(1<<10);
    uint64_t total_size = atoi(argv[3])*(1<<20);
    char *path = argv[4];

    if(io_method == "posix") {
        labstor::PosixIO *test = new labstor::PosixIO();
        //test->Init(path, block_size, total_size);
        io_test(test);
    }
    if(io_method == "mq") {
        LABSTOR_ERROR_HANDLE_START()
        labstor::LabStorMQ *test = new labstor::LabStorMQ();
        //test->Init(path, block_size, total_size);
        io_test(test);
        LABSTOR_ERROR_HANDLE_END();
    }
}