//
// Created by lukemartinlogan on 8/20/21.
//

class LabStorFS_Client {
public:
    void ProcessRequest(labstor::ipc::queue_pair *qp, struct posix_request *rq, labstor::credentials *creds) {
        switch(rq->op) {
            case OPEN_RQ: {
                break;
            }
        }
    }
private:
    void Open(const char *pathname, int flags, mode_t mode) {
    }
};

LABSTOR_MODULE_CONSTRUCT(LabStorFS_Client)