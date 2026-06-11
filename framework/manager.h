#ifndef MANAGER_H
#define MANAGER_H

#include "module_interface.h"

#include <vector>

namespace framework {
class Manager {
    public:
        // Register a module into the vector
        void RegisterModule(ModuleInterface& module) {
            modules_.push_back(&module);  // Take address of reference, store as pointer
        }

        // Call Update() on all registered modules (one cycle)
        void UpdateAll() {
            for (std::size_t i = 0; i < modules_.size(); ++i) {
                modules_[i]->Update();  // Polymorphic call via virtual function
            }
        }

    private:
        // All module types inherit ModuleInterface, so we store ModuleInterface*
        std::vector<ModuleInterface*> modules_;
};
}

#endif
