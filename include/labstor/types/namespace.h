//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_NAMESPACE_H
#define LABSTOR_NAMESPACE_H

namespace labstor {

class Namespace {
private:
    std::unordered_map<std::string, int> namespace_pkgs_; //Namespace Key -> Module ID
    std::unordered_map<std::string, void*> namespace_data_; //Namespace Key -> Data

public:
    inline bool HasKey(std::string key) {
        return namespace_pkgs_.find(key) != namespace_pkgs.end();
    }

    labstor::Module *GetModule() {
    }
};

}

#endif //LABSTOR_NAMESPACE_H
