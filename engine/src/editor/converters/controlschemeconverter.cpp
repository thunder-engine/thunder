#include "converters/controlschemeconverter.h"

#include <QFile>

#include <bson.h>
#include <json.h>

namespace {
    const char *gData("Data");
}

ControlScehemeConverterSettings::ControlScehemeConverterSettings() {
    setType(MetaType::type<ControlScheme *>());
}

AssetConverter::ReturnCode ControlSchemeConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source().data());
    if(src.open(QIODevice::ReadOnly)) {
        ControlScheme *scheme = Engine::loadResource<ControlScheme>(settings->destination());
        if(scheme == nullptr) {
            scheme = Engine::objectCreate<ControlScheme>();
        }

        VariantMap map;
        map[gData] = Json::load(src.readAll().toStdString());
        scheme->loadUserData(map);

        src.close();

        QFile file(settings->absoluteDestination().data());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(Engine::toVariant(scheme));
            file.write(reinterpret_cast<const char *>(data.data()), data.size());
            file.close();
            return Success;
        }
    }
    return InternalError;
}

AssetConverterSettings *ControlSchemeConverter::createSettings() {
    return new ControlScehemeConverterSettings();
}
