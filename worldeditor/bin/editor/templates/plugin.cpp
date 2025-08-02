#ifndef PLUGINTEMPLATE_H
#define PLUGINTEMPLATE_H

#include <module.h>

//+{Includes}
//-{Includes}

static const char *meta = \
"{"
"   \"module\": \"Module${projectName}\","
"   \"version\": \"${projectVersion}\","
"   \"description\": \"${projectName}\","
"   \"author\": \"${companyName}\","
"   \"components\": ["
        //+{ComponentNames}
        //-{ComponentNames}
"   ]"
"}";

class Module${projectName} : public Module {
public:
    Module${projectName}(Engine *engine) :
            Module(engine) {
        //+{RegisterComponents}
        //-{RegisterComponents}
    }

    ~Module${projectName}() {
        //+{UnregisterComponents}
        //-{UnregisterComponents}
    }

    const char *metaInfo() const override {
        return meta;
    }
};
#ifdef SHARED_DEFINE
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine) {
        return new Module${projectName}(engine);
    }
}
#endif
#endif // PLUGINTEMPLATE_H
