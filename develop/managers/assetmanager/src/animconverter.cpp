#include "animconverter.h"

#include <QFile>

#include <json.h>
#include <bson.h>

#define TRACKS  "Tracks"

VariantMap AnimationClipSerial::saveUserData() const {
    VariantMap result;

    VariantList tracks;
    for(auto t : m_Tracks) {
        VariantList track;
        track.push_back(t.path);
        track.push_back(t.property);

        VariantList curves;
        for(auto c : t.curves) {
            VariantList curve;
            curve.push_back(c.first);

            for(auto it : c.second.m_Keys) {
                VariantList key;
                key.push_back(int32_t(it.m_Position));
                key.push_back(it.m_Type);
                key.push_back(it.m_Value);
                key.push_back(it.m_LeftTangent);
                key.push_back(it.m_RightTangent);

                curve.push_back(key);
            }
            curves.push_back(curve);
        }
        track.push_back(curves);

        tracks.push_back(track);
    }

    result[TRACKS] = tracks;
    return result;
}

uint8_t AnimConverter::convertFile(IConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        AnimationClipSerial clip;
        VariantMap map;
        map[TRACKS] = Json::load(src.readAll().toStdString()).toList();
        clip.loadUserData(map);
        src.close();

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data  = Bson::save( Engine::toVariant(&clip) );
            file.write(reinterpret_cast<const char *>(&data[0]), data.size());
            file.close();
            return 0;
        }
    }

    return 1;
}
