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
        void RegisterModule(ModuleInterface& module) {
            modules_.push_back(&module);
        }

        void UpdateAll() {
            // first method
            // for (std::size_t i = 0; i < modules_.size(); ++i) {
            //     modules_[i]->Update();
            // }

            // second method
            for (ModuleInterface* module : modules_) {
                module->Update();
            }
        }

        
        
    private:
        std::vector<ModuleInterface*> modules_;  
};
}   // namespace framework

#endif
