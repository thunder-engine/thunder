#include "converters/animconverter.h"

#include <json.h>
#include <file.h>

#define FORMAT_VERSION 3

namespace {
    const char *gTracks("Tracks");
}

AnimImportSettings::AnimImportSettings() {
    setType(MetaType::type<AnimationClip *>());
    setVersion(FORMAT_VERSION);
}

bool AnimImportSettings::isReadOnly() const {
    return false;
}

TString AnimImportSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/anim.svg";
}

AssetConverter::ReturnCode AnimConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        AnimationClip *clip = Engine::loadResource<AnimationClip>(settings->destination());
        if(clip == nullptr) {
            clip = Engine::objectCreate<AnimationClip>();
        }

        VariantMap map;
        map[gTracks] = readJson(src.readAll(), settings).toList();
        clip->loadUserData(map);
        src.close();

        return settings->saveBinary(clip);
    }

    return InternalError;
}

AssetConverterSettings *AnimConverter::createSettings() {
    return new AnimImportSettings();
}

Variant AnimConverter::readJson(const TString &data, AssetConverterSettings *settings) {
    Variant result = Json::load(data);

    bool update = false;
    switch(settings->currentVersion()) {
        case 0: toVersion1(result); update = true;
        case 2: toVersion3(result); update = true;
        default: break;
    }

    if(update) {
        File src(settings->source());
        if(src.open(File::WriteOnly)) {
            src.write(Json::save(result, 0));
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
                QString property(prop.first.data());

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
