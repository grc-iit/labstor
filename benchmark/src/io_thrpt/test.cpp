//
// Created by lukemartinlogan on 12/31/21.
//

#include <omp.h>
#include "generator/sequential.h"
#include "posix.h"
#include "posix_aio.h"
#include "labstor_mq.h"
#include "labstor_bio.h"
#include "io_uring.h"
#include "libaio.h"
#include "spdk.h"
#include <labstor/userspace/util/partitioner.h>


void io_test(labstor::IOTest *test, const std::string &readwrite);

void io_test(labstor::IOTest *test, const std::string &readwrite) {
    printf("BATCHES PER THREAD: %lu\n", test->GetBatchesPerThread());
    printf("OPS PER BATCH: %lu\n", test->GetOpsPerBatch());
    printf("Block Size Bytes: %lu\n", test->GetBlockSizeBytes());
    labstor::ProcessAffiner affiner;
    omp_set_dynamic(0);
#pragma omp parallel num_threads(test->GetNumThreads())
    {
        int tid = labstor::ThreadLocal::GetTid();
        affiner.SetCpu(affiner.GetNumCPU() - tid - 1);
        if(readwrite == "read") {
            printf("Read Test\n");
            for (size_t i = 0; i < test->GetBatchesPerThread(); ++i) {
                test->GetTimer().Resume(tid);
                test->Read();
                test->GetTimer().Pause(tid);
            }
        } else {
            printf("Write Test\n");
            for (size_t i = 0; i < test->GetBatchesPerThread(); ++i) {
                test->GetTimer().Resume(tid);
                test->Write();
                test->GetTimer().Pause(tid);
            }
        }
    }
    printf("Total IO (MB): %lu MB\n", test->GetTotalIOBytes() / (1<<20));
    printf("Total Time: %lf us\n", test->GetTimer().GetUsec());
    printf("%s Bandwidth: %lf MBps\n", readwrite.c_str(), test->GetTotalIOBytes()/test->GetTimer().GetUsec());
    printf("%s Throughput: %lf MOps\n", readwrite.c_str(), test->GetTotalNumOps()/test->GetTimer().GetUsec());
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
    printf("Selected I/O Method: %s\n", io_method.c_str());
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
        LABSTOR_ERROR_HANDLE_START()
        labstor::LabStorBIO *test_impl = new labstor::LabStorBIO();
        generator->SetOffsetUnit(512);
        test_impl->Init(path, generator);
        test = test_impl;
        LABSTOR_ERROR_HANDLE_END();
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
    io_test(test, read_or_write);
}