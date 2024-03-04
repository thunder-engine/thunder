#include "converters/stylesheetconverter.h"

#include <QFile>

#include <bson.h>

#define FORMAT_VERSION 1

StyleSheetConverterSettings::StyleSheetConverterSettings() {
    setType(MetaType::type<StyleSheet *>());
    setVersion(FORMAT_VERSION);
}

QString StyleSheetConverterSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/css.svg";
}

AssetConverter::ReturnCode StyleSheetConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        StyleSheet style;
        QByteArray array = src.readAll();
        src.close();
        if(!array.isEmpty()) {
            style.setData(array.data());
        }

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save( Engine::toVariant(&style) );
            file.write(reinterpret_cast<const char *>(data.data()), data.size());
            file.close();
            return Success;
        }
    }

    return InternalError;
}

AssetConverterSettings *StyleSheetConverter::createSettings() const {
    return new StyleSheetConverterSettings();
}
