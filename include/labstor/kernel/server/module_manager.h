//
// Created by lukemartinlogan on 9/17/21.
//

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
