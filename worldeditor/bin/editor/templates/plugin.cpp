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
