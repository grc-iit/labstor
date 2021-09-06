//
// Created by lukemartinlogan on 8/4/21.
//

#ifndef LABSTOR_SERVER_H
#define LABSTOR_SERVER_H

void submit_request(struct request_queue *proc, void *req);
void submit_request(struct request_queue *proc, void *req);
void register_device(char *path, char *io_scheduler);
void register_io_lib(char *path);

#endif //LABSTOR_SERVER_H
