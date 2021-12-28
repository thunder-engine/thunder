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
    \fn const char *Module::metaInfo() const

    Returns a meta information in JSON format.
*/

/*!
    \fn System *Module::system(const char *name)

    Returns a module's System given \a name if present; otherwise returns nullptr.
*/

/*!
    \fn AssetConverter *Module::assetConverter(const char *name)

    Returns a module's asset converter with given \a name if present; otherwise returns nullptr.
*/

/*!
    \fn AssetEditor *Module::assetEditor()

    Returns a module's asset editor with given \a name if present; otherwise returns nullptr.
*/
