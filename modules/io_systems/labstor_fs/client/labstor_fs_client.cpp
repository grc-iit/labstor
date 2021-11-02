//
// Created by lukemartinlogan on 8/20/21.
//

class LabStorFS_Client {
public:
    void proccess_request(struct posix_request *rq) {
        switch(rq->op) {
            case OPEN_RQ: {
                break;
            }
        }
    }
private:
};

labstor::Module pkg = {
    .module_id = "ABCDE",
    .process_request_fn = nullptr,
    .get_ops = nullptr,
} module_;