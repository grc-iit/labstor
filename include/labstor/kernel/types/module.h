
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

#ifndef LABSTOR_MODULE_KERNEL_H
#define LABSTOR_MODULE_KERNEL_H

#include <labstor/types/basics.h>
#include "labstor/types/data_structures/c/shmem_queue_pair.h"
#include <labstor/types/data_structures/shmem_request.h>

typedef void (*process_request_fn_type)(struct labstor_queue_pair *qp, struct labstor_request *rq);
typedef void (*process_request_fn_netlink_type)(int pid, void *rq);

struct labstor_module {
    struct labstor_id module_id;
    labstor_runtime_id_t runtime_id;
    process_request_fn_type process_request_fn;
    process_request_fn_netlink_type process_request_fn_netlink;
};

#endif //LABSTOR_MODULE_KERNEL_H