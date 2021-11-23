//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_POSIX_CLIENT_H
#define LABSTOR_POSIX_CLIENT_H

namespace labstor::Client {

class GenericPosix : public labstor::Module {
    GenericPosix() : labstor::Module("GENERIC_POSIX") {}
};
LABSTOR_MODULE_CREATE(GenericPosix);

}

#endif //LABSTOR_POSIX_CLIENT_H
