/**
 * @file module_interface.h
 * @brief ModuleInterface for arduion_uno in framework
 * @date 11.06.2026
 */

#ifndef MODULE_INTERFACE_H
#define MODULE_INTERFACE_H

namespace framework {
class ModuleInterface {
    public:
        virtual ~ModuleInterface() {}

        virtual void Update() = 0;

        virtual void Init() {}
};
}

#endif
