#include "converters/fontconverter.h"

#include <QFile>

#include <bson.h>

#define DATA    "Data"

#define FORMAT_VERSION 1

FontImportSettings::FontImportSettings() {
    setType(MetaType::type<Font *>());
    setVersion(FORMAT_VERSION);
}

QString FontImportSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/font.svg";
}

AssetConverter::ReturnCode FontConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        Font font;

        QByteArray data = src.readAll();
        src.close();

        ByteArray m_Data;
        m_Data.resize(data.size());
        memcpy(&m_Data[0], data.data(), data.size());

        VariantMap map;
        map[DATA] = m_Data;
        font.loadUserData(map);

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save( Engine::toVariant(&font) );
            file.write((const char *)&data[0], data.size());
            file.close();
            return Success;
        }
    }
    return InternalError;
}

AssetConverterSettings *FontConverter::createSettings() const {
    return new FontImportSettings();
}
