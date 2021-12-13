//
// Created by lukemartinlogan on 10/29/21.
//

#ifndef LABSTOR_KERNEL_SERVER_H
#define LABSTOR_KERNEL_SERVER_H

#define KERNEL_SERVER

void labstor_msg_trusted_server(void *serialized_buf, size_t buf_size, int pid);

#endif //LABSTOR_KERNEL_SERVER_H
