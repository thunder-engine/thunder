#include "animconverter.h"

#include <QFile>

#include <json.h>
#include <bson.h>

#define TRACKS  "Tracks"

class AnimationClipSerial : public AnimationClip {
public:
    VariantMap                  saveUserData                () const {
        VariantMap result;

        VariantList tracks;
        for(auto t : m_Tracks) {
            VariantList track;
            track.push_back(t.path);
            track.push_back(t.property);

            VariantList keys;
            for(auto c : t.curve) {
                VariantList key;
                key.push_back(int32_t(c.mPosition));
                key.push_back(c.mType);
                key.push_back(c.mValue);
                key.push_back(c.mSupport);

                keys.push_back(key);
            }
            track.push_back(keys);

            tracks.push_back(track);
        }

        result[TRACKS]  = tracks;

        return result;
    }

protected:
    friend class AnimConverter;

};

uint8_t AnimConverter::convertFile(IConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        AnimationClipSerial clip;
        VariantMap map;
        map["Tracks"]   = Json::load(src.readAll().toStdString()).toList();
        clip.loadUserData(map);
        src.close();

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data  = Bson::save( Engine::toVariant(&clip) );
            file.write((const char *)&data[0], data.size());
            file.close();
            return 0;
        }
    }

    return 1;
}
