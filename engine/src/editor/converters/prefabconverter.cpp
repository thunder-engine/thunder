#include "converters/prefabconverter.h"

#include "components/actor.h"

#include <QFile>

#include <json.h>
#include <file.h>

#include <editor/pluginmanager.h>
#include <editor/projectsettings.h>

#define FORMAT_VERSION 5

PrefabConverterSettings::PrefabConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList PrefabConverterSettings::typeNames() const {
    return { "Prefab" };
}

bool PrefabConverterSettings::isReadOnly() const {
    return false;
}

TString PrefabConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/prefab.svg";
}

AssetConverterSettings *PrefabConverter::createSettings() {
    return new PrefabConverterSettings();
}

TString PrefabConverter::templatePath() const {
    return ":/Templates/Prefab.fab";
}

void PrefabConverter::createFromTemplate(const TString &destination) {
    QFile src(templatePath().data());
    if(src.open(QFile::ReadOnly)) {
        TString data = src.readAll().toStdString();
        src.close();

        Variant variant = Json::load(data);

        std::unordered_map<uint32_t, uint32_t> idPairs;

        VariantList &objects = *(reinterpret_cast<VariantList *>(variant.data()));
        for(auto &object : objects) {
            VariantList &o = *(reinterpret_cast<VariantList *>(object.data()));
            if(o.size() >= 5) {
                auto i = o.begin();
                ++i; // type
                uint32_t id = static_cast<uint32_t>((*i).toInt());
                uint32_t newId = ObjectSystem::generateUUID();
                idPairs[id] = newId;
                (*i) = newId;
                ++i;
                auto it = idPairs.find(static_cast<uint32_t>((*i).toInt()));
                if(it != idPairs.end()) {
                    (*i) = it->second;
                }
            }
        }

        File dst(destination);
        if(dst.open(File::WriteOnly)) {
            dst.write(Json::save(variant));
            dst.close();
        }
    }
}

void PrefabConverter::makePrefab(Actor *actor, AssetConverterSettings *settings) {
    File file(settings->source());
    if(file.open(File::WriteOnly)) {
        Prefab *fab = Engine::objectCreate<Prefab>(settings->destination());
        fab->setActor(actor);

        file.write(Json::save(Engine::toVariant(fab), 0));
        file.close();

        settings->saveSettings();
    }
}

Actor *PrefabConverter::createActor(const AssetConverterSettings *settings, const TString &guid) const {
    PROFILE_FUNCTION();

    Prefab *prefab = Engine::loadResource<Prefab>(guid);
    if(prefab && prefab->actor()) {
        return static_cast<Actor *>(prefab->actor()->clone());
    }
    return AssetConverter::createActor(settings, guid);
}

AssetConverter::ReturnCode PrefabConverter::convertFile(AssetConverterSettings *settings) {
    PROFILE_FUNCTION();

    AssetConverter::ReturnCode result = InternalError;

    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        Variant variant = readJson(src.readAll(), settings);
        src.close();

        return settings->saveBinary(variant, settings->absoluteDestination());
    }
    return result;
}

Variant PrefabConverter::readJson(const TString &data, AssetConverterSettings *settings) {
    PROFILE_FUNCTION();

    Variant result = Json::load(data);

    bool update = false;
    switch(settings->currentVersion()) {
        case 0: update |= toVersion1(result);
        case 1: update |= toVersion2(result);
        case 2: update |= toVersion3(result);
        case 3: update |= toVersion4(result);
        case 4: update |= toVersion5(result);
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

bool PrefabConverter::toVersion1(Variant &variant) {
    PROFILE_FUNCTION();

    // Create all declared objects
    VariantList &objects = *(reinterpret_cast<VariantList *>(variant.data()));
    for(auto &object : objects) {
        VariantList &o = *(reinterpret_cast<VariantList *>(object.data()));
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
                TString property(prop.first);

                property.replace("_Rotation", "quaternion");
                property.replace("Use_Kerning", "kerning");
                property.replace("Audio_Clip", "clip");
                property.remove('_');

                property[0] = std::tolower(property.at(0));

                propertiesNew[property] = prop.second;
            }
            properties = propertiesNew;
        }
    }
    return true;
}

bool PrefabConverter::toVersion2(Variant &variant) {
    return false;
}

bool PrefabConverter::toVersion3(Variant &variant) {
    return false;
}

bool PrefabConverter::toVersion4(Variant &variant) {
    return false;
}

bool PrefabConverter::toVersion5(Variant &variant) {
    VariantList &objects = *(reinterpret_cast<VariantList *>(variant.data()));

    VariantList prefab;
    prefab.push_back("Prefab");
    prefab.push_back(ObjectSystem::generateUUID());
    prefab.push_back(0);
    prefab.push_back("");
    prefab.push_back(VariantMap());
    prefab.push_back(VariantList());

    VariantList actor = *(reinterpret_cast<VariantList *>(objects.front().data()));
    auto it = actor.begin();
    ++it;

    int32_t uuid = (*it).toInt();

    VariantMap fields;
    fields["Actor"] = uuid;
    prefab.push_back(fields);

    objects.push_front(prefab);

    return true;
}
