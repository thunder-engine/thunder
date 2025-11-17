#include "buildtools.h"

#include <engine.h>

#include "converter/vsbuilder.h"
#include "converter/androidbuilder.h"
#include "converter/xcodebuilder.h"
#include "converter/linuxbuilder.h"

static const char *meta = \
"{"
"   \"module\": \"BuildTools\","
"   \"version\": \"1.0\","
"   \"description\": \"Build Tools plugin\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"AndroidBuilder\": \"converter\","
"       \"XcodeBuilder\": \"converter\","
"       \"VsBuilder\": \"converter\","
"       \"LinuxBuilder\": \"converter\""
"   }"
"}";

Module *moduleCreate(Engine *engine) {
    return new BuildTools(engine);
}

BuildTools::BuildTools(Engine *engine) :
        Module(engine) {
}

const char *BuildTools::metaInfo() const {
    return meta;
}

void *BuildTools::getObject(const char *name) {
#ifdef Q_OS_WINDOWS
    if(TString("VsBuilder") == name) {
        static VsBuilder *builder = nullptr;
        if(builder == nullptr) {
            builder = new VsBuilder;
        }
        return builder;
    }
#endif
#ifdef Q_OS_OSX
    if(TString("XcodeBuilder") == name) {
        static XcodeBuilder *builder = nullptr;
        if(builder == nullptr) {
            builder = new XcodeBuilder;
        }
        return builder;
    }
#endif
#ifdef Q_OS_LINUX
    if(TString("LinuxBuilder") == name) {
        static LinuxBuilder *builder = nullptr;
        if(builder == nullptr) {
            builder = new LinuxBuilder;
        }
        return builder;
    }
#endif
    if(TString("AndroidBuilder") == name) {
        static AndroidBuilder *builder = nullptr;
        if(builder == nullptr) {
            builder = new AndroidBuilder;
        }
        return builder;
    }

    return nullptr;
}
