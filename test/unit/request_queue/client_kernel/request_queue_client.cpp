
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/data_structures/mpmc/shmem_request_queue.h>
#include <labstor/util/singleton.h>
#include <labmods/secure_shmem/netlink_client/secure_shmem_client_netlink.h>
#include "request_queue_test.h"

int main(int argc, char **argv) {
    /*int region_id;
    size_t region_size = 4096;
    labstor::ipc::request_map q;
    auto netlink_client__ = LABSTOR_KERNEL_CLIENT;
    labstor::kernel::netlink::ShmemClient shmem_netlink;
    struct simple_request* rq;

    //Create SHMEM region
    netlink_client__->Connect();
    region_id = shmem_netlink.CreateShmem(region_size, true);
    if(region_id < 0) {
        printf("Failed to allocate SHMEM!\n");
        exit(1);
    }
    printf("Sending ID: %d\n", region_id);

    //Grant process access to region
    shmem_netlink.GrantPidShmem(getpid(), region_id);
    void *region = shmem_netlink.MapShmem(region_id, region_size);
    if(region == NULL) {
        printf("Failed to map shmem\n");
        exit(1);
    }
    printf("Mapped Region ID: %d %p\n", region_id, region);

    //Initialize request queue and place request in the queue
    q.Init(region, region_size, sizeof(simple_request));
    for(int i = 0; i < 10; ++i) {
        rq = (struct simple_request*)q.Allocate(sizeof(struct simple_request));
        rq->data = 12341 + i;
        printf("ENQUEUE IN USER: %d\n", rq->data);
        q.Enqueue(&rq->header);
    }

    //Send dequeue message to kernel module
    int code;
    struct request_queue_test_request_netlink dqmsg;
    strcpy(dqmsg.header.module_id.key, REQUEST_QUEUE_TEST_ID);
    dqmsg.rq.op = REQUEST_QUEUE_TEST_PKG_DEQUEUE;
    dqmsg.rq.region_id = region_id;
    netlink_client__->SendMSG(&dqmsg, sizeof(dqmsg));
    netlink_client__->RecvMSG(&code, sizeof(code));
    printf("Received the following code: %d\n", code);

    //Dequeue messages from kernel module
    while(rq = (struct simple_request*)q.Dequeue()) {
        printf("DEQUEUE IN USER: %d\n", rq->data);
    }

    //Delete SHMEM region
    //shmem_netlink.FreeShmem(region_id);*/
}