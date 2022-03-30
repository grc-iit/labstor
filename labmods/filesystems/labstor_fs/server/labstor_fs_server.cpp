//
// Created by lukemartinlogan on 8/20/21.
//

#include <labmods/filesystems/generic_posix/generic_posix.h>
#include <labmods/filesystems/labstor_fs/labstor_fs.h>
#include <labmods/filesystems/labstor_fs/server/labstor_fs_server.h>
#include <labmods/storage_api/generic_block/generic_block.h>
#include <list>

bool labstor::LabFS::Server::ProcessRequest(labstor::queue_pair *qp, labstor::ipc::request *request, labstor::credentials *creds) {
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
inline void labstor::LabFS::Server::Initialize(labstor::ipc::request *rq) {
    labstor::queue_pair *priv_qp;
    labstor::GenericBlock::io_request *block_rq;
    labstor::ipc::qtok_t qtok;

    register_request *reg_rq = reinterpret_cast<register_request*>(rq);
    next_module_ = namespace_->Get(reg_rq->next_);

    //Read log starting at first block
    LogCommit *commit;
    std::list<LogCommit*> log;
    Block block = log_.GetLogBlock();
    ipc_manager_->GetQueuePair(priv_qp, LABSTOR_QP_PRIVATE | LABSTOR_QP_LOW_LATENCY);
    do {
        //Load log block from storage
        commit = reinterpret_cast<LogCommit*>(malloc(block.size_));
        block_rq = ipc_manager_->AllocRequest<labstor::GenericBlock::io_request>(priv_qp);
        block_rq->Start(next_module_, labstor::GenericBlock::Ops::kRead, block.off_, block.size_, commit);
        priv_qp->Enqueue<labstor::GenericBlock::io_request>(block_rq, qtok);
        block_rq = ipc_manager_->Wait<labstor::GenericBlock::io_request>(qtok);

        //Replay log transactions

        //Get next block to load
        block = commit->next_;

        //Free block request
        ipc_manager_->FreeRequest<labstor::GenericBlock::io_request>(priv_qp, block_rq);
        log.emplace_back(commit);
    } while(block.size_);
}
inline bool labstor::LabFS::Server::Open(labstor::queue_pair *qp, labstor::GenericPosix::open_request *client_rq, labstor::credentials *creds) {
    int prior_off = 0;
    uint64_t cur_uuid = 0;
    if(client_rq->oflags_ & O_CREAT) {
        //Allocate file UUID
        log_.GetCoreLog().GetUUID();
        //Get UUID of parent directory (traverse private inode map)
        while(0) {
            int len = GetNextSlash(client_rq->path_, prior_off);
            if(len == 0) { break; }
            Inode *inode = log_->FindInode(cur_uuid, client_rq->path_, prior_off, len);
            prior_off = len+1; //index of thing after the slash
        }
        //Create new Inode
        log_->CreateInode();
        //Append to log
        log_.GetCoreLog().LogModify(client_rq);
    }
    return true;
}
inline bool labstor::LabFS::Server::Close(labstor::queue_pair *qp, labstor::GenericPosix::close_request *client_rq, labstor::credentials *creds) {
    //sync all data & metadata back to storage
    return true;
}
inline bool labstor::LabFS::Server::IO(labstor::queue_pair *qp, labstor::GenericPosix::io_request *client_rq, labstor::credentials *creds) {
    labstor::GenericBlock::io_request *block_rq;
    labstor::queue_pair *priv_qp;
    Block block;

    int i = 0;
    char *buf = reinterpret_cast<char*>(client_rq->buf_);
    size_t total_io = client_rq->size_;
    int num_blocks = (total_io/SMALL_BLOCK_SIZE) + 1;
    labstor::ipc::qtok_t *qtoks = new labstor::ipc::qtok_t[num_blocks];

    //For a read, we must identify the set of blocks
    //For a write, we must allocate new blocks

    switch(client_rq->GetCode()) {
        //Divide I/O into blocks
        case 0: {
            ipc_manager_->GetQueuePair(priv_qp, LABSTOR_QP_PRIVATE | LABSTOR_QP_LOW_LATENCY);
            for (size_t cur_io = 0; cur_io < total_io; ++i) {
                size_t io_size = (total_io - cur_io < LARGE_BLOCK_SIZE) ? SMALL_BLOCK_SIZE : LARGE_BLOCK_SIZE;
                switch(static_cast<labstor::GenericPosix::Ops>(client_rq->op_)) {
                    case labstor::GenericPosix::Ops::kWrite: {
                        log_.GetCoreLog().GetBlock(io_size, block);
                        break;
                    }
                    case labstor::GenericPosix::Ops::kRead: {
                        break;
                    }
                }
                block_rq = ipc_manager_->AllocRequest<labstor::GenericBlock::io_request>(priv_qp);
                block_rq->Start(next_module_, static_cast<labstor::GenericBlock::Ops>(client_rq->op_), block.off_, block.size_, buf);
                priv_qp->Enqueue(block_rq, qtoks[i]);
                buf += io_size;
            }
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
                log_.GetCoreLog().LogModify(block_rq);
                ++client_rq->cur_qtok_;
            }
            return true;
        }
    }
    return true;
}

LABSTOR_MODULE_CONSTRUCT(labstor::LabFS::Server, LABFS_MODULE_ID)