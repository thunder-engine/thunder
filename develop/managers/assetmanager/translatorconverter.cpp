#include "translatorconverter.h"

#include <QFile>

#include <bson.h>
#include "resources/text.h"
#include "projectmanager.h"

class TranslatorSerial : public Translator {
public:
    map<string, string> m_Data;

protected:
    VariantMap saveUserData () const {
        VariantMap result;
        VariantMap data;

        for(auto it : m_Data) {
            data[it.first] = it.second;
        }

        result["Data"]  = data;
        return result;
    }
};

uint8_t TranslatorConverter::convertFile(IConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        TranslatorSerial loc;

        while(!src.atEnd()) {
            QByteArray line = src.readLine();
            auto split = line.split(';');
            loc.m_Data[split.first().constData()] = split.last().constData();
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

IConverterSettings *TranslatorConverter::createSettings() const {
    IConverterSettings *result = IConverter::createSettings();
    result->setType(MetaType::type<Translator *>());
    return result;
}
