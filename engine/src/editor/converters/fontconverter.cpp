#include "converters/fontconverter.h"

#include <QFile>

#include <bson.h>

namespace {
    const char *gData("Data");
}

#define FORMAT_VERSION 1

FontImportSettings::FontImportSettings() {
    setType(MetaType::type<Font *>());
    setVersion(FORMAT_VERSION);
}

TString FontImportSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/font.svg";
}

AssetConverter::ReturnCode FontConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source().data());
    if(src.open(QIODevice::ReadOnly)) {
        Font *font = Engine::loadResource<Font>(settings->destination());
        if(font == nullptr) {
            font = Engine::objectCreate<Font>();
        }

        QByteArray data = src.readAll();
        src.close();

        ByteArray m_Data;
        m_Data.resize(data.size());
        memcpy(&m_Data[0], data.data(), data.size());

        VariantMap map;
        map[gData] = m_Data;
        font->loadUserData(map);

        QFile file(settings->absoluteDestination().data());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save( Engine::toVariant(font) );
            file.write(reinterpret_cast<const char *>(data.data()), data.size());
            file.close();
            return Success;
        }
    }
    return InternalError;
}

AssetConverterSettings *FontConverter::createSettings() {
    return new FontImportSettings();
}
