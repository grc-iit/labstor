//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_IPCMANAGER_H
#define LABSTOR_IPCMANAGER_H

#include <labstor/types/basics.h>

namespace labstor {

struct LabStorServerContext;

class IPCManager {
private:
    pthread_t thread_;
public:
    void* WreapProcesses(void*);
    void CreateIPC(int client_fd, labstor::credentials *creds, int count, size_t size);
};

}
#endif //LABSTOR_IPCMANAGER_H
