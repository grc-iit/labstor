
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

#ifndef LABSTOR_CONSTANTS_H
#define LABSTOR_CONSTANTS_H

#define LABSTOR_REQUEST_SUCCESS 0
#define LABSTOR_REQUEST_FAILED -1000

#define LABSTOR_INVALID_NAMESPACE_KEY -1

#define LABSTOR_REGISTRAR_ID 0

enum {
    IPC_TEST_MODULE_RUNTIME_ID,
    SHMEM_MODULE_RUNTIME_ID,
    IPC_MANAGER_MODULE_RUNTIME_ID,
    WORKER_MODULE_RUNTIME_ID,
    MQ_DRIVER_RUNTIME_ID,
    BLKDEV_TABLE_RUNTIME_ID
};

#endif //LABSTOR_CONSTANTS_H