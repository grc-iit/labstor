//
// Created by lukemartinlogan on 11/23/21.
//

#ifndef LABSTOR_ROUND_ROBIN_CLIENT_H
#define LABSTOR_ROUND_ROBIN_CLIENT_H

namespace labstor::iosched::Manual {

class RoundRobin : public labstor::Module {
private:
    int hi;
public:
    RoundRobin() : labstor::Module("RoundRobin") {}

    void Write(void *buf, size_t buf_size, size_t lba, int dev_id) {
    }

    void Read(void *buf, size_t buf_size, size_t lba, int dev_id) {
    }
};

}

#endif //LABSTOR_ROUND_ROBIN_CLIENT_H
