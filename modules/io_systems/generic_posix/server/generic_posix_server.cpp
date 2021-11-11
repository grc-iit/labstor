//
// Created by lukemartinlogan on 8/20/21.
//

#include <string.h>

#include "../include/generic_posix/posix.h"
#include <labstor/types/ipc_pool.h>
#include <labstor/util/singleton.h>

class GenericFS_Server {
private:
    std::unordered_map<int, int> client_to_server_fd_;
    std::unordered_map<int, void*> fd_to_md_;

public:
    static void process_request(labstor::queue_pair *qp, struct posix_request *rq, labstor::credentials *creds) {
        switch(rq->op) {
            case POSIX_OPEN_RQ:
                Open(qp, creds, rq->open.pathname, rq->open.flags, rq->open.mode);
                break;
            case POSIX_CLOSE_RQ:
                break;
        }
    }

private:
    inline int GetParentDir(const  char *pathaname, int cur_size) {
        for(int i = cur_size; i >= 0; --i) {
            if(pathname[i] == '/') {
                return i;
            }
        }
    }

    inline void Open(labstor::queue_pair *qp, labstor::credentials *creds, const char *pathname, int flags, mode_t mode) {
        int pathlen = strlen(pathname);
        int dev_pathlen = pathlen;
        auto namespace_ = &Singleton<labstor::Server::ConfigurationManager>->namespace_;
        struct posix_ops *ops;
        labstor::Module *pkg;
        void *data;
        do {
            std::string new_path = std::string(pathname, dev_pathlen);
            if(namespace_.HasKey(new_path)) {
                namespace_.GetData(new_path, pkg, data);
                ops = (struct posix_ops *)pkg->get_ops();
                ops->open(qp, creds, data, pathname, flags, mode);
            }
        } while((dev_pathlen = GetParentDir(pathname, dev_pathlen)) != 0);
    }

    inline void Write(int fd, ) {

    }

    inline void Close(int fd) {
    }
};

labstor::Module module_ = {
    .process_request_fn = GenericFS_Server::process_request,
    .get_ops = nullptr
};
