#include "translatorconverter.h"

#include <QFile>

#include <bson.h>

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
    AssetConverterSettings *result = AssetConverter::createSettings();
    result->setType(MetaType::type<Translator *>());
    return result;
}
