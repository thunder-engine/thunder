#include "fontconverter.h"

#include <QFile>

#include <bson.h>

#include <file.h>

#include "resources/font.h"
#include "projectmanager.h"

#define HEADER  "Header"
#define DATA    "Data"

class FontSerial : public Font {
public:
    void                        setData         (const QByteArray &data) {
        m_Data.resize(data.size());
        memcpy(&m_Data[0], data.data(), data.size());
    }

protected:
    VariantMap                  saveUserData    () const {
        VariantMap result;
        {
            VariantList header;
            header.push_back(0); // Reserved
            header.push_back(m_Scale);
            header.push_back(m_FontName);
            result[HEADER]  = header;
        }
        {
            result[DATA]    = m_Data;
        }
        return result;
    }
};

uint8_t FontConverter::convertFile(IConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        FontSerial font;
        font.setData(src.readAll());
        src.close();

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data  = Bson::save( Engine::toVariant(&font) );
            file.write((const char *)&data[0], data.size());
            file.close();
            return 0;
        }
    }
    return 1;
}
