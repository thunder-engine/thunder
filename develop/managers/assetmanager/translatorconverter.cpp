#include "translatorconverter.h"

#include <QFile>

#include <bson.h>
#include "resources/text.h"
#include "projectmanager.h"

uint8_t TranslatorConverter::convertFile(AssetConverterSettings *settings) {
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
            ByteArray data  = Bson::save( Engine::toVariant(&loc) );
            file.write(reinterpret_cast<const char *>(&data[0]), data.size());
            file.close();
            return 0;
        }
    }

    return 1;
}

AssetConverterSettings *TranslatorConverter::createSettings() const {
    AssetConverterSettings *result = AssetConverter::createSettings();
    result->setType(MetaType::type<Translator *>());
    return result;
}
