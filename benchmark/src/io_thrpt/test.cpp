//
// Created by lukemartinlogan on 12/31/21.
//

#include "generator/sequential.h"
#include "posix.h"
#include "posix_aio.h"
#include "labstor_mq.h"
#include "io_uring.h"
#include "libaio.h"
#include "spdk.h"

void write_test(labstor::IOTest *test);
void read_test(labstor::IOTest *test);

void write_test(labstor::IOTest *test) {
    labstor::HighResMonotonicTimer t;
    printf("BATCHES PER THREAD: %lu\n", test->GetBatchesPerThread());
    printf("OPS PER BATCH: %lu\n", test->GetOpsPerBatch());
    printf("Block Size Bytes: %lu\n", test->GetBlockSizeBytes());
    for(size_t i = 0; i < test->GetBatchesPerThread(); ++i) {
        t.Resume();
        test->Write();
        t.Pause();
    }
    printf("Total IO (MB): %lu MB\n", test->GetTotalIOBytes() / (1<<20));
    printf("Write Bandwidth: %lf MBps\n", test->GetTotalIOBytes()/t.GetUsec());
    printf("Write Throughput: %lf MOps\n", test->GetTotalNumOps()/t.GetUsec());
}

void read_test(labstor::IOTest *test) {
    labstor::HighResMonotonicTimer t;
    printf("BATCHES PER THREAD: %lu\n", test->GetBatchesPerThread());
    printf("OPS PER BATCH: %lu\n", test->GetOpsPerBatch());
    printf("Block Size Bytes: %lu\n", test->GetBlockSizeBytes());
    for(size_t i = 0; i < test->GetBatchesPerThread(); ++i) {
        t.Resume();
        test->Read();
        t.Pause();
    }
    printf("Total IO (MB): %lu\n", test->GetTotalIOBytes() / (1<<20));
    printf("Read Bandwidth: %lf MBps\n", test->GetTotalIOBytes()/t.GetUsec());
    printf("Read Throughput: %lf MOps\n", test->GetTotalNumOps()/t.GetUsec());
}

int main(int argc, char **argv) {
    if(argc != 9) {
        printf("USAGE: ./io_test [io_method] [r/w] [truncate (yes/no)] [block_size_kb] [total_size_mb] [queue_depth] [nthreads] [path]");
        exit(1);
    }
    std::string io_method = std::string(argv[1]);
    std::string read_or_write = std::string(argv[2]);
    bool truncate = std::string(argv[3]) == "yes";
    uint64_t block_size = ((uint64_t)atoi(argv[4]))*(1<<10);
    uint64_t total_size = ((uint64_t)atoi(argv[5]))*(1<<20);
    int ops_per_batch = atoi(argv[6]);
    int nthreads = atoi(argv[7]);
    char *path = argv[8];

    //Workload generator
    labstor::SequentialGenerator *generator = new labstor::SequentialGenerator();
    generator->Init(block_size, total_size, ops_per_batch, nthreads);

    //I/O Mechanisms
    labstor::IOTest *test = nullptr;
    if(io_method == "posix") {
        labstor::PosixIO *test_impl = new labstor::PosixIO();
        generator->SetOffsetUnit(1);
        test_impl->Init(path, truncate, generator);
        test = test_impl;
    }
    else if(io_method == "posix_aio") {
        labstor::PosixAIO *test_impl = new labstor::PosixAIO();
        generator->SetOffsetUnit(1);
        test_impl->Init(path, truncate, generator);
        test = test_impl;
    }
    else if(io_method == "io_uring") {
        labstor::IOUringIO *test_impl = new labstor::IOUringIO();
        generator->SetOffsetUnit(1);
        test_impl->Init(path, truncate, generator);
        test = test_impl;
    }
    else if(io_method == "libaio") {
        labstor::LibAIO *test_impl = new labstor::LibAIO();
        generator->SetOffsetUnit(1);
        test_impl->Init(path, truncate, generator);
        test = test_impl;
    }
    else if(io_method == "spdk") {
        labstor::SPDKIO *test_impl = new labstor::SPDKIO();
        generator->SetOffsetUnit(512);
        test_impl->Init(generator);
        test = test_impl;
    }
    else if(io_method == "dax") {
    }
    else if(io_method == "mq") {
        LABSTOR_ERROR_HANDLE_START()
        labstor::LabStorMQ *test_impl = new labstor::LabStorMQ();
        generator->SetOffsetUnit(512);
        test_impl->Init(path, generator);
        test = test_impl;
        LABSTOR_ERROR_HANDLE_END();
    }
    else if(io_method == "bio") {
    }
    else if(io_method == "labfs") {
    }
    else if(io_method == "labkvs") {
    }
    else {
        printf("%s did not match any io_methods\n", io_method.c_str());
        exit(1);
    }

    //IO Tests
    if(test == NULL) {
        printf("%s was not implemented\n", io_method.c_str());
        exit(1);
    }
    if(read_or_write == "write") {
        write_test(test);
    }
    else if(read_or_write == "read") {
        read_test(test);
    }
}