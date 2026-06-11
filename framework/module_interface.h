#ifndef MODULE_INTERFACE_H
#define MODULE_INTERFACE_H

namespace framework {
class ModuleInterface {
    public:
        // Virtual destructor: ensures correct cleanup when deleting via base pointer
        virtual ~ModuleInterface() {}

        // Pure virtual function: all subclasses must implement Update()
        virtual void  Init() {};

        virtual void Update() = 0;
};
}

#endif
