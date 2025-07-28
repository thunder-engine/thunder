#include "converters/textconverter.h"

#include <QFile>

#include <bson.h>
#include "resources/text.h"

#define FORMAT_VERSION 1

TextConverterSettings::TextConverterSettings() {
    setType(MetaType::type<Text *>());
    setVersion(FORMAT_VERSION);
}

TString TextConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/text.svg";
}

AssetConverter::ReturnCode TextConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source().data());
    if(src.open(QIODevice::ReadOnly)) {
        Text text;
        QByteArray array = src.readAll();
        src.close();
        if(!array.isEmpty()) {
            text.setSize(array.size());
            memcpy(text.data(), array.data(), array.size());
        }

        QFile file(settings->absoluteDestination().data());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save( Engine::toVariant(&text) );
            file.write(reinterpret_cast<const char *>(data.data()), data.size());
            file.close();
            return Success;
        }
    }

    return InternalError;
}

AssetConverterSettings *TextConverter::createSettings() {
    return new TextConverterSettings();
}
