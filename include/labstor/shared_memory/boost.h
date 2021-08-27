//
// Created by lukemartinlogan on 8/23/21.
//

#ifndef LABSTOR_BOOST_H
#define LABSTOR_BOOST_H

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <labstor/shared_memory/shared_memory.h>

namespace labstor::ipc {

struct boost_shmem : public SharedMemory {
    boost::interprocess::shared_memory_object shmem_obj_;
    boost::interprocess::mapped_region shmem_;
    std::string path_;

    void init(std::string path, size_t size) {
        shmem_obj_ = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, path.c_str(), boost::interprocess::read_write);
        shmem_obj_.truncate(size);
        shmem_ = boost::interprocess::mapped_region(shmem_obj_, boost::interprocess::read_write);
        path_ = path;
    }

    void open_rw(std::string path) {
        shmem_obj_ = boost::interprocess::shared_memory_object(boost::interprocess::open_only, path.c_str(), boost::interprocess::read_write);
        shmem_ = boost::interprocess::mapped_region(shmem_obj_, boost::interprocess::read_write);
        path_ = path;
    }

    void open_ro(std::string path) {
        shmem_obj_ = boost::interprocess::shared_memory_object(boost::interprocess::open_only, path.c_str(), boost::interprocess::read_only);
        shmem_ = boost::interprocess::mapped_region(shmem_obj_, boost::interprocess::read_only);
        path_ = path;
    }

    void remove() {
        boost::interprocess::shared_memory_object::remove(path_.c_str());
    }

    void remove(std::string path) {
        boost::interprocess::shared_memory_object::remove(path.c_str());
    }

    void *get_address() {
        return shmem_.get_address();
    }
};

}

#endif //LABSTOR_BOOST_H
