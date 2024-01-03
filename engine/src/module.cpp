#include "module.h"

/*!
    \class Module
    \brief A base interface for all engine modules.
    \inmodule Engine

    Thunder Engine uses modules to extend engine capabilities.
    Every engine module must be inherited from the Module class.
    This class contains basic information about the content of the module and creates some of the necessary in-module systems.
*/

/*!
    Returns a module manifest in JSON format.
*/
const char *Module::metaInfo() const {
    return nullptr;
}

/*!
    This function is a facory for the module. It return a pointer to constructed object with given type \a name.
*/
void *Module::getObject(const char *name) {
    A_UNUSED(name);
    return nullptr;
}
