/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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
