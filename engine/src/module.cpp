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
    \enum Module::PluginTypes

    \value SYSTEM \c Module provides a system.
    \value EXTENSION \c Module provides a set of components.
    \value CONVERTER \c Module provides a converter for assets (only in Editor).
    \value EDITOR \c Module provides an asset editor (only in Editor).
*/

/*!
    \fn const char *Module::description() const

    Returns a short description of the module.
*/

/*!
    \fn const char *Module::version() const

    Return an internal version of the module as a string.
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

/*!
    \fn StringList Module::components() const

    Returns a list of provided components.
*/
