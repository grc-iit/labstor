//
// Created by lukemartinlogan on 12/5/21.
//

void labstor::MQDriver::Client::IO(int ns_id, Ops op, int dev_id, char *user_buf, size_t lba, size_t off, int hctx) {
    AUTO_TRACE("labstor::MQDriver::Client::IO", rq_submit->path_);
    labstor_complete_blkdev_table_register_request *rq_complete;
    labstor_submit_blkdev_table_register_request *kern_submit;
    labstor_complete_blkdev_table_register_request *kern_complete;
    labstor::ipc::queue_pair *kern_qp;
    labstor::ipc::qtok_t qtok;

    //Get KERNEL QP
    ipc_manager_->GetQueuePair(kern_qp,
                               LABSTOR_QP_SHMEM | LABSTOR_QP_STREAM | LABSTOR_QP_INTERMEDIATE | LABSTOR_QP_ORDERED | LABSTOR_QP_LOW_LATENCY,
                               0, KERNEL_PID);

    //Create Client -> KERNEL message
    kern_submit = reinterpret_cast<labstor_submit_blkdev_table_register_request *>(
            ipc_manager_->AllocRequest(kern_qp, sizeof(labstor_submit_blkdev_table_register_request)));
    kern_submit->Init(BLKDEV_TABLE_RUNTIME_ID, rq_submit);
    dev_ids_.Dequeue(kern_submit->dev_id_);
    TRACEPOINT("KERN SUBMIT", kern_submit->path_);

    //Complete Client -> KERNEL interaction
    qtok = kern_qp->Enqueue(reinterpret_cast<labstor::ipc::request*>(kern_submit));
    kern_complete = reinterpret_cast<labstor_complete_blkdev_table_register_request *>(ipc_manager_->Wait(qtok));

    //Create message for the USER
    rq_complete = reinterpret_cast<labstor_complete_blkdev_table_register_request *>(
            ipc_manager_->AllocRequest(qp, sizeof(labstor_complete_blkdev_table_register_request)));
    rq_complete->header.ns_id_ = -1;

    //Complete Client -> USER interaction
    TRACEPOINT("labstor::MQDriver::Client::AddBdev", "Kernel Complete", (size_t)kern_complete, (int)kern_complete->header.ns_id_);
    qp->Complete(
            reinterpret_cast<labstor::ipc::request*>(rq_submit),
            reinterpret_cast<labstor::ipc::request*>(rq_complete));
}

labstor::ipc::qtok_t labstor::MQDriver::Client::AIO(int ns_id, Ops op, int dev_id, char *user_buf, size_t lba, size_t off, int hctx) {
}


void labstor::MQDriver::Client::UnmountDriver(labstor::ipc::queue_pair *qp, labstor_submit_blkdev_table_register_request *rq_submit) {
    AUTO_TRACE("labstor::MQDriver::Client::RemoveBdev")
}