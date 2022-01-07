//
// Created by lukemartinlogan on 12/31/21.
//

#include "posix.h"
#include "posix_aio.h"
#include "labstor_mq.h"
#include "io_uring.h"

void write_test(labstor::IOTest *test) {
    labstor::HighResMonotonicTimer t;
    for(size_t i = 0; i < test->GetBatchesPerThread(); ++i) {
        t.Resume();
        test->Write();
        t.Pause();
    }
    printf("Total IO (MB): %lu MB\n", test->GetTotalIO() / (1<<20));
    printf("Write Bandwidth: %lf MBps\n", test->GetTotalIO()/t.GetUsec());
    printf("Write Throughput: %lf KOps\n", test->GetTotalNumOps()/t.GetMsec());
}

void read_test(labstor::IOTest *test) {
    labstor::HighResMonotonicTimer t;
    for(size_t i = 0; i < test->GetBatchesPerThread(); ++i) {
        t.Resume();
        test->Read();
        t.Pause();
    }
    printf("Total IO (MB): %lu\n", test->GetTotalIO() / (1<<20));
    printf("Read Bandwidth: %lf MBps\n", test->GetTotalIO()/t.GetUsec());
    printf("Read Throughput: %lf KOps\n", test->GetTotalNumOps()/t.GetMsec());
}

int main(int argc, char **argv) {
    if(argc != 9) {
        printf("USAGE: ./io_test [io_method] [r/w] [truncate (yes/no)] [block_size_kb] [total_size_mb] [queue_depth] [nthreads] [path]");
        exit(1);
    }
    std::string io_method = std::string(argv[1]);
    std::string read_or_write = std::string(argv[2]);
    bool truncate = std::string(argv[3]) == "yes";
    uint64_t block_size = atoi(argv[4])*(1<<10);
    uint64_t total_size = atoi(argv[5])*(1<<20);
    int queue_depth = atoi(argv[6]);
    int nthreads = atoi(argv[7]);
    char *path = argv[8];

    labstor::IOTest *test = nullptr;
    if(io_method == "posix") {
        labstor::PosixIO *test_impl = new labstor::PosixIO();
        test_impl->Init(path, block_size, total_size, queue_depth, nthreads, truncate);
        test = test_impl;
    }
    else if(io_method == "posix_aio") {
        labstor::PosixAIO *test_impl = new labstor::PosixAIO();
        test_impl->Init(path, block_size, total_size, queue_depth, nthreads, truncate);
        test = test_impl;
    }
    else if(io_method == "iouring") {
        labstor::IOUringIO *test_impl = new labstor::IOUringIO();
        test_impl->Init(path, block_size, total_size, queue_depth, nthreads, truncate);
        test = test_impl;
    }

    else if(io_method == "mq") {
        LABSTOR_ERROR_HANDLE_START()
        labstor::LabStorMQ *test_impl = new labstor::LabStorMQ();
        test_impl->Init(path, block_size, total_size, queue_depth, nthreads);
        test = test_impl;
        LABSTOR_ERROR_HANDLE_END();
    }
    else if(io_method == "bio") {
    }
    else {
        printf("%s did not match any io_methods\n", io_method.c_str());
        exit(1);
    }

    if(read_or_write == "write") {
        write_test(test);
    }
    else if(read_or_write == "read") {
        read_test(test);
    }
}