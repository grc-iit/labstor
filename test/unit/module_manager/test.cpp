
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

#include <labstor/userspace/client/client.h>
#include <labstor/userspace/server/module_manager.h>

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
    labstor::Module *simple_module = modules.GetModuleConstructor(SIMPLE_MODULE_ID)();
    printf("%s\n", simple_module->GetModuleID().key_);
    LABSTOR_ERROR_HANDLE_END()
}