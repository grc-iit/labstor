
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

#ifndef LABSTOR_DUMMY_H
#define LABSTOR_DUMMY_H

#include <labstor/types/data_structures/shmem_request.h>

#define LABSTOR_DUMMY_MODULE_ID "Dummy"

namespace labstor::test::Dummy {

enum class Ops {
    kInit,
    kGetValue=4
};

struct dummy_request : public labstor::ipc::request {
    int value_;
    dummy_request() {}
    void Start(uint32_t ns_id) {
        ns_id_ = ns_id;
        op_ = static_cast<int>(Ops::kGetValue);
        code_ = 0;
    }
    void Complete(uint32_t value) {
        value_ = value;
    }
};

}

#endif //LABSTOR_DUMMY_H