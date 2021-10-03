#include "mapconverter.h"

#include <QFile>

#include <bson.h>
#include <json.h>
#include <map.h>
#include <components/actor.h>

#define FORMAT_VERSION 2

MapConverterSettings::MapConverterSettings() {
    setType(MetaType::type<Actor *>());
    setVersion(FORMAT_VERSION);
}

QString MapConverterSettings::typeName() const {
    return "Map";
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
