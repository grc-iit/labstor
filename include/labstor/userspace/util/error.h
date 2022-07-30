
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
#define LABSTOR_ERROR_PTR err
#define LABSTOR_ERROR_HANDLE_CATCH catch(LABSTOR_ERROR_TYPE &LABSTOR_ERROR_PTR)
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