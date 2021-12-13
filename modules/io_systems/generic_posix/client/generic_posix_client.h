//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_POSIX_CLIENT_H
#define LABSTOR_POSIX_CLIENT_H

namespace labstor::GenericPosix {
class Client {
private:
    int hi;
public:
    void Write();
    void Read();
};
}

#endif //LABSTOR_POSIX_CLIENT_H
