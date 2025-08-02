#include <engine.h>

#include <file.h>
#include <log.h>
#include <platform.h>

//+{ModuleIncludes}
//-{ModuleIncludes}

#include "plugin.cpp"

int thunderMain(Engine *engine) {
    Log::setLogLevel(Log::DBG);

    if(engine->init()) {
        //+{RegisterModules}
        //-{RegisterModules}
        engine->addModule(new Module${projectName}(engine));

        engine->start();
    }

    return 0;
}

THUNDER_MAIN()
