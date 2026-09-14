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
#include "angel.h"

#include "angelsystem.h"

#include <cstring>

#ifdef SHARED_DEFINE
#include "converters/angelbuilder.h"

Module *moduleCreate(Engine *engine) {
    return new Angel(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"Angel\","
"   \"version\": \"1.0\","
"   \"description\": \"Angel Script Module\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"AngelSystem\": \"system\","
"       \"AngelBuilder\": \"converter\""
"   },"
"   \"components\": ["
"       \"AngelBehaviour\""
"   ]"
"}";

Angel::Angel(Engine *engine) :
        Module(engine),
        m_pSystem(new AngelSystem(engine)) {
}

Angel::~Angel() {
    delete m_pSystem;
}

const char *Angel::metaInfo() const {
    return meta;
}

void *Angel::getObject(const char *name) {
    if(strcmp(name, "AngelSystem") == 0) {
        return m_pSystem;
    }
#ifdef SHARED_DEFINE
    else if(strcmp(name, "AngelBuilder") == 0) {
        static AngelBuilder *builder = nullptr;
        if(builder == nullptr) {
            builder = new AngelBuilder(m_pSystem);
        }
        return builder;
    }
#endif
    return nullptr;
}
