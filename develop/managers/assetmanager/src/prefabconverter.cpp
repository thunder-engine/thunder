#include "prefabconverter.h"

#include <QFile>

#include <bson.h>
#include <json.h>
#include <components/actor.h>

#define DATA    "Data"

uint8_t PrefabConverter::convertFile(IConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        Variant actor   = Json::load(src.readAll().toStdString());
        src.close();

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data  = Bson::save( actor );
            file.write((const char *)&data[0], data.size());
            file.close();
            return 0;
        }
    }
    return 1;
}
