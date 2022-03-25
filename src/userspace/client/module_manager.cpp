//
// Created by lukemartinlogan on 11/4/21.
//

#include <labstor/userspace/client/client.h>
#include <labstor/types/basics.h>
#include <labstor/userspace/types/module.h>
#include <labstor/userspace/util/errors.h>
#include <labstor/userspace/client/macros.h>
#include <labstor/userspace/client/ipc_manager.h>
#include <labstor/userspace/client/namespace.h>
#include <labstor/userspace/client/module_manager.h>
#include <labmods/registrar/client/registrar_client.h>

labstor::id labstor::Client::ModuleManager::UpdateModule(std::string path) {
    AUTO_TRACE(path)
    labstor::id module_id;
    labstor::ModuleHandle module_info;
    labstor::Module *old_instance, *new_instance;
    labstor_runtime_id_t runtime_id;
    LABSTOR_NAMESPACE_T namespace_ = LABSTOR_NAMESPACE;
    LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;

    //Pause all queues & wait until there are no busy queues
    ipc_manager_->PauseQueues();
    ipc_manager_->WaitForPause();

    //Process update
    LABSTOR_ERROR_HANDLE_TRY {
        module_info = OpenModule(path, module_id);
        TRACEPOINT("MODULE_ID", module_id.key_)
        /*std::queue<labstor::Module*> &modules = namespace_->AllModuleInstances(module_id);
        TRACEPOINT("NAMESPACE KEYS", modules.size())
        for(int i = 0; i < modules.size(); ++i) {
            labstor::Module *old_instance = modules.front();
            modules.pop();
            new_instance = module_info.constructor_();
            new_instance->StateUpdate(old_instance);
            modules.push(new_instance);
            delete old_instance;
        }*/
        SetModuleConstructor(module_id, module_info);
        return module_id;
    } LABSTOR_ERROR_HANDLE_CATCH {
        ipc_manager_->ResumeQueues();
        throw err;
    }
}
