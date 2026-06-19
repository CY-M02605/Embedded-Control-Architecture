/**
 * @file manager.h
 * @brief Declares the Manager for Arduino UNO.
 * @date 2026-06-11
 */

#ifndef MANAGER_H
#define MANAGER_H

#include "module_interface.h"

namespace framework {
class Manager {
    public:
        Manager() : count_(0) {}

        void RegisterModule(ModuleInterface& module) {
            if (count_ < MAX_MODULES) {
                modules_[count_] = &module;
                count_++;
            }
        }

        void UpdateAll() {
            for (int i = 0; i < count_; ++i) {
                modules_[i]->Update();
            }
        }


    private:
        static const int MAX_MODULES = 8;
        ModuleInterface* modules_[MAX_MODULES];
        int count_;
};
}

#endif
