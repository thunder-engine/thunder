#ifndef PLUGINTEMPLATE_H
#define PLUGINTEMPLATE_H

#include <module.h>

${Includes}

class ${Project_Name} : public IModule {
public:
    ${Project_Name}             (Engine *engine) {
        ObjectSystem system;
        ${RegisterComponents}
    }

    ~${Project_Name}            () {
        ObjectSystem system;
        ${UnregisterComponents}
    }

    const char                 *description             () const {
        return "${Project_Name}";
    }

    const char                 *version                 () const {
        return "${Project_Version}";
    }

    uint8_t                     types                   () const {
        return EXTENSION;
    }

    StringList                  components              () const {
        StringList result;
        ${ComponentNames}
        return result;
    }
};
#ifdef NEXT_SHARED
extern "C" {
    MODULE_EXPORT IModule *moduleCreate(Engine *engine) {
        return new ${Project_Name}(engine);
    }
}
#endif
#endif // PLUGINTEMPLATE_H
