//
// Created by lukemartinlogan on 8/20/21.
//

int labstor_fs_open(const char *pathname, int flags, mode_t mode) {
    if(!namespace_) {
        namespace_ = boost::interprocess::shared_memory_object(boost::interprocess::create, "LabStorNamespace", boost::interprocess::read_or_write)
    }
    int pathlen = sizeof(pathname);
    for(int i = pathlen - 1; i >= 0; --i) {

    }
}