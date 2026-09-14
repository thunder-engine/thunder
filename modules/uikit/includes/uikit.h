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
#ifndef UIKIT_H
#define UIKIT_H

#include <module.h>

#if defined(SHARED_DEFINE) && defined(_WIN32)
    #ifdef UIKIT_LIBRARY
        #define UIKIT_EXPORT __declspec(dllexport)
    #else
        #define UIKIT_EXPORT __declspec(dllimport)
    #endif
#else
    #define UIKIT_EXPORT
#endif

class UiSystem;

class UIKIT_EXPORT UiKit : public Module {
public:
    UiKit(Engine *engine);
    ~UiKit();

    const char *metaInfo() const override;

    void *getObject(const char *name) override;

};
#ifdef SHARED_DEFINE
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // UIKIT_H
