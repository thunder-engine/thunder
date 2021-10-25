#include "mapconverter.h"

#include <QFile>

#include <bson.h>
#include <json.h>
#include <map.h>
#include <components/actor.h>

#define FORMAT_VERSION 3

MapConverterSettings::MapConverterSettings() {
    setType(MetaType::type<Scene *>());
    setVersion(FORMAT_VERSION);
}

QString MapConverterSettings::typeName() const {
    return "Map";
}

bool MapConverterSettings::isReadOnly() const {
    return false;
}

uint8_t MapConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        string data = src.readAll().toStdString();
        src.close();

        Variant actor = readJson(data, settings);
        injectResource(actor, Engine::objectCreate<Map>(""));

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(actor);
            file.write((const char *)&data[0], data.size());
            file.close();

            return 0;
        }
    }
    return 1;
}

AssetConverterSettings *MapConverter::createSettings() const {
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
            string type = i->toString();
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
