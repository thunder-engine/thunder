#include "converters/animconverter.h"

#include <QFile>

#include <json.h>
#include <bson.h>

#define TRACKS  "Tracks"

#define FORMAT_VERSION 3

AnimImportSettings::AnimImportSettings() {
    setType(MetaType::type<AnimationClip *>());
    setVersion(FORMAT_VERSION);
}

bool AnimImportSettings::isReadOnly() const {
    return false;
}

QString AnimImportSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/anim.svg";
}

AssetConverter::ReturnCode AnimConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        AnimationClip clip;
        VariantMap map;
        map[TRACKS] = readJson(src.readAll().toStdString(), settings).toList();
        clip.loadUserData(map);
        src.close();

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save( Engine::toVariant(&clip) );
            file.write(reinterpret_cast<const char *>(&data[0]), data.size());
            file.close();
            return Success;
        }
    }

    return InternalError;
}

AssetConverterSettings *AnimConverter::createSettings() {
    return new AnimImportSettings();
}

Variant AnimConverter::readJson(const std::string &data, AssetConverterSettings *settings) {
    Variant result = Json::load(data);

    bool update = false;
    switch(settings->currentVersion()) {
        case 0: toVersion1(result); update = true;
        case 2: toVersion3(result); update = true;
        default: break;
    }

    if(update) {
        QFile src(settings->source());
        if(src.open(QIODevice::WriteOnly)) {
            std::string data = Json::save(result, 0);
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

void AnimConverter::toVersion3(Variant &variant) {
    PROFILE_FUNCTION();

    // Create all declared objects
    for(auto &trackIt : *(reinterpret_cast<VariantList *>(variant.data()))) {
        VariantList &trackData = *(reinterpret_cast<VariantList *>(trackIt.data()));
        auto i = trackData.begin();

        i++;
        i++;
        i++;

        for(auto &it : (*i).toList()) {
            VariantList &curveList = *(reinterpret_cast<VariantList *>(it.data()));
            auto t = curveList.begin();

            curveList.remove(*t);
        }
    }
}
