//
// Created by lukemartinlogan on 8/20/21.
//

class LabStorFSServer {
private:
    std::unordered_map<int, int> client_to_server_fd_;
    std::unordered_map<int, void*> fd_to_md_;

public:
    static void process_request(labstor::ipc::queue_pair *qp, struct posix_request *rq, labstor::credentials *creds) {
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

    inline void Open(labstor::ipc::queue_pair *qp, labstor::credentials *creds, const char *pathname, int flags, mode_t mode) {

    }

    inline void Write(int fd, ) {

    }

    inline void Close(int fd) {
    }
};