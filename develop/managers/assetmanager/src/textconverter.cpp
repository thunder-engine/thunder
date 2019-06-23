#include "textconverter.h"

#include <QFile>

#include <bson.h>
#include "resources/text.h"
#include "projectmanager.h"

class TextSerial : public Text {
public:
    void setData (const QByteArray &array) {
        if(!array.isEmpty()) {
            setSize(array.size());
            memcpy(data(), array.data(), array.size());
        }
    }

protected:
    VariantMap saveUserData () const {
        VariantMap result;
        result["Data"]  = data();
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
