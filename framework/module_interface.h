/**
 * @file module_interafce.h
 * @brief Defines the common interface used by the manager to initialize and execute modules
 * @date 2026-06-11
 */

#ifndef MODULE_INTERFACE_H
#define MODULE_INTERFACE_H

namespace framework {
class ModuleInterface {
    public:
        // Virtual destructor: ensures correct cleanup when deleting via base pointer
        virtual ~ModuleInterface() {};

        virtual void Init() {};

        // Pure virtual function: all subclasses must implement Update()
        virtual void Update() = 0;

};
}

#endif
