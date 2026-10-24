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
#include <cstring>

#include "uikit.h"

#include "converters/stylesheetconverter.h"
#include "converters/uiconverter.h"

namespace {
    const char *meta =
        "{"
        "   \"module\": \"UiKitForms\","
        "   \"version\": \"1.0\","
        "   \"description\": \"UiKit Form Editor\","
        "   \"author\": \"Evgeniy Prikazchikov\","
        "   \"objects\": {"
        "       \"UiEdit\": \"editor\","
        "       \"UiConverter\": \"converter\","
        "       \"StyleSheetConverter\": \"converter\""
        "   }"
        "}";
}

class UiKitForms : public Module {
public:
    explicit UiKitForms(Engine *engine) :
            Module(engine) {
    }

    const char *metaInfo() const override {
        return meta;
    }

    void *getObject(const char *name) override {
        if(std::strcmp(name, "UiConverter") == 0) {
            return new UiConverter;
        } else if(std::strcmp(name, "StyleSheetConverter") == 0) {
            return new StyleSheetConverter;
        }
        return nullptr;
    }
};

Module *moduleCreate(Engine *engine) {
    return new UiKitForms(engine);
}
