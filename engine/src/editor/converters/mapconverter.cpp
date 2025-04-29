#include "converters/mapconverter.h"

#include <map.h>

#define FORMAT_VERSION 5

MapConverterSettings::MapConverterSettings() {
    setType(MetaType::type<Scene *>());
    setVersion(FORMAT_VERSION);
}

QStringList MapConverterSettings::typeNames() const {
    return { "Map" };
}

bool MapConverterSettings::isReadOnly() const {
    return false;
}

QString MapConverterSettings::defaultIconPath(const QString &) const {
    return ":/Style/styles/dark/images/map.svg";
}

AssetConverterSettings *MapConverter::createSettings() {
    return new MapConverterSettings();
}

QString MapConverter::templatePath() const {
    return QString();
}

bool MapConverter::toVersion3(Variant &variant) {
    VariantList &objects = *(reinterpret_cast<VariantList *>(variant.data()));
    int32_t root = 0; // First object is a root
    for(auto object = objects.begin(); object != objects.end(); ++object) {
        VariantList &o = *(reinterpret_cast<VariantList *>(object->data()));
        if(o.size() >= 5) {
            auto i = o.begin();
            std::string type = i->toString();
            if(root == 0) {
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
    VariantList &objects = *(reinterpret_cast<VariantList *>(variant.data()));
    for(auto object = objects.begin(); object != objects.end(); ++object) {
        VariantList &o = *(reinterpret_cast<VariantList *>(object->data()));
        if(o.size() >= 5) {
            auto i = o.begin();
            std::string type = i->toString();
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
