/**
 * @file manager.h
 * @brief Defines the manager responsible for module registration and cyclic execution
 * @date 2026-06-11
 */

#ifndef MANAGER_H
#define MANAGER_H

#include "module_interface.h"

#include <vector>

namespace framework {
class Manager {
    public:

        // Register a module by storing its address in the module container.
        void RegisterModule(ModuleInterface& module) {
            modules_.push_back(&module);
        }

        // Call UpdateAll() for every registered module.
        void UpdateAll() {
            // Index-based iteration
            // for (std::size_t i = 0; i < modules_.size(); ++i) {
            //     modules_[i]->Update();
            // }

            // Range-based iteration
            for (ModuleInterface* module : modules_) {
                module->Update();
            }
        }
        
    private:
        // Store pointers to all registered modules.
        std::vector<ModuleInterface*> modules_;  
};
}   // namespace framework

#endif
