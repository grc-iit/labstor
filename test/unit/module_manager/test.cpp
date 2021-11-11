//
// Created by lukemartinlogan on 11/2/21.
//

#include <labstor/userspace_server/module_manager.h>

const labstor::id SIMPLE_MODULE_ID = "SIMPLE_MODULE";

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("USAGE: ./test [module_path.so]\n");
        exit(1);
    }
    LABSTOR_ERROR_HANDLE_START()
    char *path = argv[1];
    labstor::Server::ModuleManager modules;
    modules.UpdateModule(path);
    labstor::Module *simple_module = modules.GetModule(SIMPLE_MODULE_ID);
    printf("%s\n", simple_module->GetModuleID().key);
    LABSTOR_ERROR_HANDLE_END()
}