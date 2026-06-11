/**
 * @file manager.h
 * @brief Manager for arduino_uno in framework
 * @date 11.06 2026
 */

#ifndef MANAGER_H
#define MANAGER_H

#include "module_interface.h"

namespace framework {
class Manager {
    public:
        Manager() : count_(0) {}

        void RegisterModule(ModuleInterface* module) {
            if (count_ < MAX_MODULES) {
                modules_[count_] = &module;
                count_++;
            }
        }

        void UpdateAll() {
            for (int i = 0; i < MAX_MODULES; ++i) {
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
