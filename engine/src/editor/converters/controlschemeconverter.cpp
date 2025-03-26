#include "converters/controlschemeconverter.h"

#include <QFile>

#include <bson.h>
#include <json.h>

ControlScehemeConverterSettings::ControlScehemeConverterSettings() {
    setType(MetaType::type<ControlScheme *>());
}

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
            file.write(reinterpret_cast<const char *>(data.data()), data.size());
            file.close();
            return Success;
        }
    }
    return InternalError;
}

AssetConverterSettings *ControlScehemeConverter::createSettings() {
    return new ControlScehemeConverterSettings();
}
