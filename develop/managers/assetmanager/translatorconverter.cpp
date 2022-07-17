#include "translatorconverter.h"

#include <QFile>

#include <bson.h>

#define FORMAT_VERSION 1

TranslatorConverterSettings::TranslatorConverterSettings() {
    setType(MetaType::type<Translator *>());
    setVersion(FORMAT_VERSION);
}

QString TranslatorConverterSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/l10n.svg";
}

AssetConverter::ReturnCode TranslatorConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        Translator loc;

        while(!src.atEnd()) {
            QByteArray line = src.readLine();
            auto split = line.split(';');
            loc.setPair(split.first().constData(), split.last().constData());
        }
        src.close();

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save( Engine::toVariant(&loc) );
            file.write(reinterpret_cast<const char *>(&data[0]), data.size());
            file.close();
            return Success;
        }
    }

    return InternalError;
}

AssetConverterSettings *TranslatorConverter::createSettings() const {
    return new TranslatorConverterSettings();
}
