
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

#ifndef LABSTOR_MODULE_MANAGER_KERNEL_H
#define LABSTOR_MODULE_MANAGER_KERNEL_H

#include <labstor/types/basics.h>
#include <labstor/kernel/types/module.h>

void init_labstor_module_manager(void);
void free_labstor_module_manager(void);
void register_labstor_module(struct labstor_module *pkg);
void unregister_labstor_module(struct labstor_module *pkg);
struct labstor_module *get_labstor_module_by_runtime_id(int runtime_id);

#endif //LABSTOR_module_manager_H