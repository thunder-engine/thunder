#include <engine.h>

#include <file.h>
#include <log.h>
#include <platform.h>

#include <rendergl.h>

#include "plugin.cpp"

int thunderMain(Engine *engine) {
    Log::setLogLevel(Log::DBG);

    if(engine->init()) {
        engine->addModule(new RenderGL(engine));
        engine->addModule(new ${Project_Name}(engine));

        engine->start();
    }

    return 0;
}

THUNDER_MAIN()
