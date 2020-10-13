#include "prefabconverter.h"

#include "components/actor.h"

#include <QFile>
#include <QDebug>

#include <bson.h>
#include <json.h>

#define FORMAT_VERSION 1

IConverterSettings *PrefabConverter::createSettings() const {
    IConverterSettings *result = IConverter::createSettings();
    result->setVersion(FORMAT_VERSION);
    return result;
}

uint8_t PrefabConverter::convertFile(IConverterSettings *settings) {
    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        string data = src.readAll().toStdString();
        src.close();

        Variant actor = readJson(data, settings);

        Object *object = Engine::toObject(actor);
        Prefab *fab = Engine::objectCreate<Prefab>("");
        fab->setActor(static_cast<Actor *>(object));

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(Engine::toVariant(fab));
            file.write((const char *)&data[0], data.size());
            file.close();

            return 0;
        }
    }
    return 1;
}

Variant PrefabConverter::readJson(const string &data, IConverterSettings *settings) {
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

void PrefabConverter::toVersion1(Variant &variant) {
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
                property.replace("Use_Kerning", "kerning");
                property.replace("Audio_Clip", "clip");
                property.replace('_', "");
                property.replace(0, 1, property[0].toLower());

                propertiesNew[property.toStdString()] = prop.second;
            }
            properties = propertiesNew;
        }
    }
}
