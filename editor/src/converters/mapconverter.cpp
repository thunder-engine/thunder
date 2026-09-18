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
#include "converters/mapconverter.h"

#include <map.h>

#define FORMAT_VERSION 5

MapConverterSettings::MapConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList MapConverterSettings::typeNames() const {
    return { MetaType::name<Map>() };
}

bool MapConverterSettings::isReadOnly() const {
    return false;
}

void MapConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/map.svg");
    }
}

AssetConverterSettings *MapConverter::createSettings() {
    return new MapConverterSettings();
}

TString MapConverter::templatePath() const {
    return TString();
}

bool MapConverter::toVersion3(Variant &variant) {
    VariantList objects = variant.toList();
    int32_t root = 0; // First object is a root
    for(auto object = objects.begin(); object != objects.end(); ++object) {
        VariantList &o = *(reinterpret_cast<VariantList *>(object->data()));
        if(o.size() >= 5) {
            auto i = o.begin();
            TString type = i->toString();
            if(root == 0 && type != "Map") {
                *i = "Chunk";
            }
            ++i;
            int32_t id = i->toInt();
            ++i;
            if(root == 0) {
                root = id;
            } else if(type == "Transform" && *i == root) {
                // Need to remove Transfrom from the Chunk
                object = objects.erase(object);
            }
        }
    }

    return true;
}

bool MapConverter::toVersion4(Variant &variant) {
    VariantList objects = variant.toList();
    for(auto object = objects.begin(); object != objects.end(); ++object) {
        VariantList &o = *(reinterpret_cast<VariantList *>(object->data()));
        if(o.size() >= 5) {
            auto i = o.begin();
            TString type = i->toString();
            if(type == "Chunk") {
                *i = "Scene";
            }
        }
    }

    return true;
}

bool MapConverter::toVersion5(Variant &variant) {
    Object *object = Engine::toObject(variant);
    if(object) {
        if(object->typeName() == "Scene") {
            Map *resource = Engine::objectCreate<Map>();

            object->setParent(resource);
            resource->setScene(dynamic_cast<Scene *>(object));

            variant = Engine::toVariant(resource);
        }
    }

    return true;
}
