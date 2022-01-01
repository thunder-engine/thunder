#ifndef PLUGINTEMPLATE_H
#define PLUGINTEMPLATE_H

#include <module.h>

//+{Includes}
//-{Includes}

static const char *meta = \
"{"
"   \"module\": \"Module${Project_Name}\","
"   \"version\": \"${Project_Version}\","
"   \"description\": \"${Project_Name}\","
"   \"components\": ["
        //+{ComponentNames}
        //-{ComponentNames}
"   ]"
"}";

class Module${Project_Name} : public Module {
public:
    Module${Project_Name}(Engine *engine) :
            Module(engine) {
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
};
#ifdef NEXT_SHARED
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine) {
        return new Module${Project_Name}(engine);
    }
}
#endif
#endif // PLUGINTEMPLATE_H
