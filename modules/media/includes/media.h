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
#ifndef MEDIA_H
#define MEDIA_H

#include <module.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef MEDIA_LIBRARY
        #define MEDIA_EXPORT __declspec(dllexport)
    #else
        #define MEDIA_EXPORT __declspec(dllimport)
    #endif
#else
    #define MEDIA_EXPORT
#endif

class Media : public Module {
public:
    Media(Engine *engine);
    ~Media();

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

protected:
    System *m_system;

};
#ifdef SHARED_DEFINE
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // MEDIA_H
