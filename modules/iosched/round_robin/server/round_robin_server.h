//
// Created by lukemartinlogan on 11/23/21.
//

#ifndef LABSTOR_ROUND_ROBIN_SERVER_H
#define LABSTOR_ROUND_ROBIN_SERVER_H

namespace labstor::Server::iosched {

class RoundRobin : public labstor::Module {
private:
    int hi;
public:
    void Write();
    void Read();
};

}

#endif //LABSTOR_ROUND_ROBIN_SERVER_H
