//
// Created by lukemartinlogan on 8/20/21.
//

#include <labmods/filesystems/generic_posix/generic_posix.h>
#include <labmods/filesystems/block_fs/block_fs.h>
#include <labmods/filesystems/block_fs/server/block_fs_server.h>
#include <labmods/storage_api/generic_block/generic_block.h>
#include <list>

bool labstor::BlockFS::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
    switch(static_cast<labstor::GenericPosix::Ops>(request->GetOp())) {
        case labstor::GenericPosix::Ops::kOpen: {
            return Open(qp, reinterpret_cast<labstor::GenericPosix::open_request*>(request), creds);
        }
        case labstor::GenericPosix::Ops::kClose: {
            return Close(qp, reinterpret_cast<labstor::GenericPosix::close_request*>(request), creds);
        }
        case labstor::GenericPosix::Ops::kWrite:
        case labstor::GenericPosix::Ops::kRead: {
            return IO(qp, reinterpret_cast<labstor::GenericPosix::io_request*>(request), creds);
        }
    }
    return true;
}
inline void labstor::BlockFS::Server::Initialize(labstor::ipc::request *rq) {
    labstor::queue_pair *priv_qp;
    labstor::GenericBlock::io_request *block_rq;
    labstor::ipc::qtok_t qtok;

    register_request *reg_rq = reinterpret_cast<register_request*>(rq);
    next_module_ = namespace_->Get(reg_rq->next_);
}
inline bool labstor::BlockFS::Server::Open(labstor::queue_pair *qp, labstor::GenericPosix::open_request *client_rq, labstor::credentials *creds) {
    return true;
}
inline bool labstor::BlockFS::Server::Close(labstor::queue_pair *qp, labstor::GenericPosix::close_request *client_rq, labstor::credentials *creds) {
    //sync all data & metadata back to storage
    return true;
}
inline bool labstor::BlockFS::Server::IO(labstor::queue_pair *qp, labstor::GenericPosix::io_request *client_rq, labstor::credentials *creds) {
    labstor::GenericBlock::io_request *block_rq;
    labstor::queue_pair *priv_qp;

    int i = 0;
    char *buf = reinterpret_cast<char*>(client_rq->buf_);
    size_t total_io = client_rq->size_;
    int num_blocks = (total_io/SMALL_BLOCK_SIZE) + 1;
    labstor::ipc::qtok_t *qtoks = new labstor::ipc::qtok_t[num_blocks];

    switch(client_rq->GetCode()) {
        //Divide I/O into blocks
        case 0: {
            ipc_manager_->GetQueuePair(priv_qp, LABSTOR_QP_PRIVATE | LABSTOR_QP_LOW_LATENCY);
            block_rq = ipc_manager_->AllocRequest<labstor::GenericBlock::io_request>(priv_qp);
            block_rq->Start(next_module_, static_cast<labstor::GenericBlock::Ops>(client_rq->op_), client_rq->off_, client_rq->size_, buf);
            priv_qp->Enqueue(block_rq, qtoks[i]);
            client_rq->SetQtoks(i, qtoks);
            client_rq->SetCode(1);
            return false;
        }

        //FS only performs direct I/O; commit blocks when I/O completes
        case 1: {
            ipc_manager_->GetQueuePair(priv_qp, client_rq->qtoks_[0]);
            for(int i = client_rq->cur_qtok_; i < client_rq->num_qtoks_; ++i) {
                if(!priv_qp->IsComplete(client_rq->qtoks_[i], block_rq)) {
                    return  false;
                }
                ++client_rq->cur_qtok_;
            }
            return true;
        }
    }
    return true;
}

LABSTOR_MODULE_CONSTRUCT(labstor::BlockFS::Server, BLOCKFS_MODULE_ID)