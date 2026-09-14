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
#include "media.h"

#include "mediasystem.h"

#include <cstring>

#ifdef SHARED_DEFINE
#include "converters/audioconverter.h"

Module *moduleCreate(Engine *engine) {
    return new Media(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"Media\","
"   \"version\": \"1.0\","
"   \"description\": \"Media Module\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"MediaSystem\": \"system\","
"       \"AudioConverter\": \"converter\""
"   },"
"   \"components\": ["
"       \"AudioSource\""
"   ]"
"}";

/*!
    \module Media

    \brief Provides audio playback and media processing functionality for Thunder Engine.
*/

Media::Media(Engine *engine) :
        Module(engine),
        m_system(nullptr) {
}

Media::~Media() {
    delete m_system;
}

const char *Media::metaInfo() const {
    return meta;
}

void *Media::getObject(const char *name) {
    if(strcmp(name, "MediaSystem") == 0) {
        if(m_system == nullptr) {
            m_system = new MediaSystem();
        }
        return m_system;
    }
#ifdef SHARED_DEFINE
    else if(strcmp(name, "AudioConverter") == 0) {
        return new AudioConverter();
    }
#endif
    return nullptr;
}
