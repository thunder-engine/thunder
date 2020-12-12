#include "textconverter.h"

#include <QFile>

#include <bson.h>
#include "resources/text.h"
#include "projectmanager.h"

uint8_t TextConverter::convertFile(IConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        Text text;
        QByteArray array = src.readAll();
        src.close();
        if(!array.isEmpty()) {
            text.setSize(array.size());
            memcpy(text.data(), array.data(), array.size());
        }

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data  = Bson::save( Engine::toVariant(&text) );
            file.write((const char *)&data[0], data.size());
            file.close();
            return 0;
        }
    }

    return 1;
}

IConverterSettings *TextConverter::createSettings() const {
    IConverterSettings *result = IConverter::createSettings();
    result->setType(MetaType::type<Text *>());
    return result;
}
