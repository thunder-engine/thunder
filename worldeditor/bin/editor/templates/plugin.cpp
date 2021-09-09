#ifndef PLUGINTEMPLATE_H
#define PLUGINTEMPLATE_H

#include <module.h>

//+{Includes}
//-{Includes}

class Module${Project_Name} : public Module {
public:
    Module${Project_Name}(Engine *engine) :
            m_pEngine(engine) {
        //+{RegisterComponents}
        //-{RegisterComponents}
    }

    ~Module${Project_Name}() {
        //+{UnregisterComponents}
        //-{UnregisterComponents}
    }

    const char *description() const override {
        return "${Project_Name}";
    }

    const char *version() const override {
        return "${Project_Version}";
    }

    int types() const override {
        return EXTENSION;
    }

    StringList components() const override {
        StringList result;
        //+{ComponentNames}
        //-{ComponentNames}
        return result;
    }

    Engine *m_pEngine;
};
#ifdef NEXT_SHARED
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine) {
        return new Module${Project_Name}(engine);
    }
}
#endif
#endif // PLUGINTEMPLATE_H
