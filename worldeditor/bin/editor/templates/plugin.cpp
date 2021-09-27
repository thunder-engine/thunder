#ifndef PLUGINTEMPLATE_H
#define PLUGINTEMPLATE_H

#include <module.h>

//+{Includes}
//-{Includes}

static const char *meta = \
"{"
"   \"version\": \"${Project_Version}\","
"   \"description\": \"${Project_Name}\","
"   \"extensions\": ["
        //+{ComponentNames}
        //-{ComponentNames}
"   ]"
"}";

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

    const char *metaInfo() const override {
        return meta;
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
