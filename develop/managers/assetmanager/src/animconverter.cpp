#include "animconverter.h"

#include <QFile>

#include <json.h>
#include <bson.h>

#define TRACKS  "Tracks"

#define FORMAT_VERSION 1

VariantMap AnimationClipSerial::saveUserData() const {
    VariantMap result;

    VariantList tracks;
    for(auto t : m_Tracks) {
        VariantList track;
        track.push_back(t.path());
        track.push_back(t.property());

        VariantList curves;
        for(auto c : t.curves()) {
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
        map[TRACKS] = readJson(src.readAll().toStdString(), settings).toList();
        clip.loadUserData(map);
        src.close();

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save( Engine::toVariant(&clip) );
            file.write(reinterpret_cast<const char *>(&data[0]), data.size());
            file.close();
            return 0;
        }
    }

    return 1;
}

Variant AnimConverter::readJson(const string &data, IConverterSettings *settings) {
    Variant result = Json::load(data);

    bool update = false;
    switch(settings->currentVersion()) {
        case 0: toVersion1(result); update = true;
        default: break;
    }

    if(update) {
        QFile src(settings->source());
        if(src.open(QIODevice::WriteOnly)) {
            settings->setCurrentVersion(settings->version());

            string data = Json::save(result, 0);
            src.write(data.c_str(), data.size());
            src.close();
        }
    }

    return result;
}

void AnimConverter::toVersion1(Variant &variant) {
    PROFILE_FUNCTION();

    // Create all declared objects
    VariantList &objects = *(reinterpret_cast<VariantList *>(variant.data()));
    for(auto &it : objects) {
        VariantList &o = *(reinterpret_cast<VariantList *>(it.data()));
        if(o.size() >= 5) {
            auto i = o.begin();
            ++i;
            ++i;
            ++i;
            ++i;

            // Load base properties
            VariantMap &properties = *(reinterpret_cast<VariantMap *>((*i).data()));
            VariantMap propertiesNew;
            for(auto &prop : properties) {
                QString property(prop.first.c_str());

                property.replace("_Rotation", "quaternion");
                property.replace(0, 1, property[0].toLower());

                propertiesNew[property.toStdString()] = prop.second;
            }
            properties = propertiesNew;
        }
    }
}
