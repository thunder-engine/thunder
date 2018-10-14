#include "textconverter.h"

#include <QFile>

#include <bson.h>
#include "resources/text.h"
#include "projectmanager.h"

class TextSerial : public Text {
public:
    void                        setData         (const QByteArray &data) {
        if(!data.isEmpty()) {
            m_Data.resize(data.size());
            memcpy(&m_Data[0], data.data(), data.size());
        }
    }

protected:
    VariantMap                  saveUserData    () const {
        VariantMap result;
        result["Data"]  = m_Data;
        return result;
    }
};

uint8_t TextConverter::convertFile(IConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        TextSerial text;
        text.setData(src.readAll());
        src.close();

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
