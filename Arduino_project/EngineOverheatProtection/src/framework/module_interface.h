/**
 * @file module_interface.h
 * @brief ModuleInterface in EngineOverheatProtection module for Arduino hardware system
 * @date 2026-07-22
 */

#ifndef MODULE_INTERFACE_H
#define MODULE_INTERFACE_H

namespace module_interafce {
class ModuleInterface {
    public:
        virtual ~ModuleInterface() {}
        
        virtual void Update() = 0;

        virtual void Init() {}

};
}

#endif
