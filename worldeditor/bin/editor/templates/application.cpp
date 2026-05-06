#include <engine.h>

#include <file.h>
#include <log.h>
#include <platform.h>

//+{ModuleIncludes}
//-{ModuleIncludes}

#include "plugin.cpp"

int thunderMain() {
    Engine::setOrganizationName(COMPANY_NAME);
    Engine::setApplicationName(PRODUCT_NAME);
    Engine::setApplicationVersion(PRODUCT_VERSION);
    Log::setLogLevel(Log::INF);

    Engine *engine = new Engine;
    if(Engine::init()) {
        //+{RegisterModules}
        //-{RegisterModules}
        Engine::addModule(new Module${projectName}(engine));

        Engine::start();
    }
    // No need to delete Engine after all

    return 0;
}

THUNDER_MAIN()
