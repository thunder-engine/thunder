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
#include "rendervk.h"

#include "rendervksystem.h"

#ifdef SHARED_DEFINE
Module *moduleCreate(Engine *engine) {
    return new RenderVK(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"RenderVK\","
"   \"version\": \"1.0\","
"   \"description\": \"Vulkan Render Module\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"dependencies\": {"
"       \"engine\": \"lib\","
"       \"next\": \"lib\""
"   },"
"   \"objects\": {"
"       \"RenderVK\": \"render\""
"   }"
"}";

RenderVK::RenderVK(Engine *engine) :
        Module(engine) {
}

RenderVK::~RenderVK() {

}

const char *RenderVK::metaInfo() const {
    return meta;
}

void *RenderVK::getObject(const char *) {
    return new RenderVkSystem(m_engine);
}
