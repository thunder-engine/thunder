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
    \fn int Module::types() const

    Returns a set of plugin types which module supports.
    To return more then one type plise use syntax like this:

    \code
    return SYSTEM | EXTENSION | CONVERTER;
    \endcode
*/

/*!
    \fn System *Module::system()

    Returns a module's System if present; otherwise returns nullptr.
*/

/*!
    \fn IConverter *Module::converter()

    Returns a module's converter if present; otherwise returns nullptr.
*/

/*!
    \fn IAssetEditor *Module::assetEditor()

    Returns a module's asset editor if present; otherwise returns nullptr.
*/

/*!
    \fn StringList Module::components() const

    Returns a list of provided components.
*/
