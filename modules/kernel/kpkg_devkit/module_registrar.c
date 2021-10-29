//
// Created by lukemartinlogan on 10/29/21.
//

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kobject.h>

#include "module_registrar.h"
#include "unordered_map.h"

struct unordered_map modules;

void init_labstor_module_registrar(int max_size) {
    unordered_map_init(&modules, max_size);
}
EXPORT_SYMBOL(init_labstor_module_registrar);

void free_labstor_module_registrar(void) {
    unordered_map_free(&modules);
}
EXPORT_SYMBOL(free_labstor_module_registrar);

void register_labstor_module(struct labstor_module *pkg) {
    unordered_map_add(&modules, pkg->module_id, pkg);
}
EXPORT_SYMBOL(register_labstor_module);

void unregister_labstor_module(struct labstor_module *pkg) {
    //unordered_map_remove(&modules, pkg->module_id);
}
EXPORT_SYMBOL(unregister_labstor_module);

struct labstor_module *get_labstor_module(struct labstor_id module_id) {
    int id;
    return unordered_map_get(&modules, module_id, &id);
}
EXPORT_SYMBOL(get_labstor_module);

struct labstor_module *get_labstor_module_by_runtime_id(int runtime_id) {
    return unordered_map_get_idx(&modules, runtime_id);
}
EXPORT_SYMBOL(get_labstor_module_by_runtime_id);