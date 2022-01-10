//
// Created by lukemartinlogan on 5/30/21.
//

#ifndef LABSTOR_ERROR_H
#define LABSTOR_ERROR_H

#ifdef __cplusplus

#include <iostream>
#include <string>
#include <cstdlib>
#include <memory>
#include <labstor/userspace/util/serializeable.h>

#define LABSTOR_ERROR_TYPE std::shared_ptr<labstor::Error>
#define LABSTOR_ERROR_HANDLE_START() try {
#define LABSTOR_ERROR_HANDLE_END() } catch(LABSTOR_ERROR_TYPE &err) { err->print(); exit(err->get_code()); }
#define LABSTOR_ERROR_HANDLE_TRY try
#define LABSTOR_ERROR_HANDLE_CATCH catch(LABSTOR_ERROR_TYPE &err)
#define LABSTOR_ERROR_IS(err, check) (err->get_code() == check.get_code())

namespace labstor {
    class Error {
    private:
        int code_;
        std::string fmt_;
        std::string msg_;
    public:
        Error() : code_(0), fmt_(nullptr) {}
        Error(int code, const std::string &fmt) : code_(code), fmt_(fmt) {}
        ~Error() {}

        int get_code() { return code_; }

        template<typename ...Args>
        std::shared_ptr<Error> format(Args ...args) const {
            std::shared_ptr<Error> err = std::make_shared<Error>(code_, fmt_);
            err->msg_ = Formatter::format(fmt_, args...);
            return err;
        }

        void print() {
            std::cout << msg_ << std::endl;
        }
    };
}

#endif

#endif //LABSTOR_ERROR_H
