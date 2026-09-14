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
#include "uikit.h"

#include <cstring>

#include "uisystem.h"

#ifdef SHARED_DEFINE
#include "editor/uiedit.h"
#include "converters/uiconverter.h"
#include "converters/stylesheetconverter.h"

Module *moduleCreate(Engine *engine) {
    return new UiKit(engine);
}
#endif

static const char *meta = \
"{"
"   \"module\": \"UiKit\","
"   \"version\": \"1.0\","
"   \"description\": \"UiKit Module\","
"   \"author\": \"Evgeniy Prikazchikov\","
"   \"objects\": {"
"       \"UiSystem\": \"system\","
"       \"UiEdit\": \"editor\","
"       \"UiConverter\": \"converter\","
"       \"StyleSheetConverter\": \"converter\""
"   },"
"   \"components\": ["
"       \"AbstractButton\","
"       \"AbstractScrollArea\","
"       \"AbstractSlider\","
"       \"Button\","
"       \"Canvas\","
"       \"CheckBox\","
"       \"FloatInput\","
"       \"Foldout\","
"       \"Frame\","
"       \"Image\","
"       \"Label\","
"       \"LineEdit\","
"       \"ListView\","
"       \"ListViewDelegate\","
"       \"TreeView\","
"       \"Menu\","
"       \"ProgressBar\","
"       \"RectTransform\","
"       \"ScrollBar\","
"       \"Slider\","
"       \"Splitter\","
"       \"Switch\","
"       \"TabBar\","
"       \"TabWidget\","
"       \"ToolButton\","
"       \"UiLoader\","
"       \"Widget\""
"   ]"
"}";

UiKit::UiKit(Engine *engine) :
        Module(engine) {

}

UiKit::~UiKit() {

}

const char *UiKit::metaInfo() const {
    return meta;
}

void *UiKit::getObject(const char *name) {
    if(strcmp(name, "UiSystem") == 0) {
        return new UiSystem;
    }
#ifdef SHARED_DEFINE
    if(strcmp(name, "UiEdit") == 0) {
        return new UiEdit;
    } else if(strcmp(name, "UiConverter") == 0) {
        return new UiConverter;
    } else if(strcmp(name, "StyleSheetConverter") == 0) {
        return new StyleSheetConverter;
    }
#endif
    return nullptr;
}
