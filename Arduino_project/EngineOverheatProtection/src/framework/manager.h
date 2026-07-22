/**
 * @file manager.h
 * @brief Manager in EngineOverheatProtection module for Arduino hardware file system
 * @date 2026-07-22
 */

#ifndef MANAGER_H
#define MANAGER_H

#include "module_interface.h"

#include <vector>

namespace manager {
class Manager {
    public:
        // void RegisterModule(module_interafce::ModuleInterface& module) {
        //     modules_.push_back(&module);
        // }

        Manager(): count_(0) {}

        void RegisterModule(module_interafce::ModuleInterface* module) {
            if (count_ < MAX_SIZE) {
                modules_[count_] = module;
                count_++;
            }
        }

        void UpdateAll() {
            for (size_t i = 0;i < count_; ++i) {
                modules_[i]->Update();
            }
        }

    private:
        // std::vector<module_interafce::ModuleInterface*> modules_;

        static const int MAX_SIZE = 10;
        module_interafce::ModuleInterface* modules_[MAX_SIZE];
        int count_;
};
}

#endif
