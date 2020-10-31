#include <engine.h>

#include <file.h>
#include <log.h>
#include <platform.h>

#include <rendergl.h>

//+{ModuleIncludes}
//-{ModuleIncludes}

#include "plugin.cpp"

int thunderMain(Engine *engine) {
    Log::setLogLevel(Log::DBG);

    if(engine->init()) {
        //+{RegisterModules}
        //-{RegisterModules}
        engine->addModule(new Module${Project_Name}(engine));

        engine->start();
    }

    return 0;
}

THUNDER_MAIN()
