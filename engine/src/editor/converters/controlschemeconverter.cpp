#include "converters/controlschemeconverter.h"

#include <QFile>

#include <bson.h>
#include <json.h>

AssetConverter::ReturnCode ControlScehemeConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        ControlScheme scheme;

        VariantMap map;
        map["Data"] = Json::load(src.readAll().toStdString());
        scheme.loadUserData(map);

        src.close();

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(Engine::toVariant(&scheme));
            file.write((const char *)&data[0], data.size());
            file.close();
            return Success;
        }
    }
    return InternalError;
}

AssetConverterSettings *ControlScehemeConverter::createSettings() {
    AssetConverterSettings *result = AssetConverter::createSettings();
    result->setType(MetaType::type<ControlScheme *>());
    return result;
}
