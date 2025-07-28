#include "converters/prefabconverter.h"

#include "components/actor.h"

#include <QFile>

#include <bson.h>
#include <json.h>

#include <editor/pluginmanager.h>
#include <editor/projectsettings.h>

#define FORMAT_VERSION 5

PrefabConverterSettings::PrefabConverterSettings() {
    setType(MetaType::type<Prefab *>());
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

        QFile dst(destination.data());
        if(dst.open(QFile::ReadWrite)) {
            data = Json::save(variant);
            dst.write(data.data(), data.size());
            dst.close();
        }
    }
}

void PrefabConverter::makePrefab(Actor *actor, AssetConverterSettings *settings) {
    QFile file(settings->source().data());
    if(file.open(QIODevice::WriteOnly)) {
        Prefab *fab = Engine::objectCreate<Prefab>("");
        fab->setActor(actor);

        TString str = Json::save(Engine::toVariant(fab), 0);
        file.write(str.data(), str.size());
        file.close();

        settings->saveSettings();
        Engine::setResource(fab, settings->destination().toStdString());
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

    QFile src(settings->source().data());
    if(src.open(QIODevice::ReadOnly)) {
        std::string data = src.readAll().toStdString();
        src.close();

        Variant variant = readJson(data, settings);
        QFile file(settings->absoluteDestination().data());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(variant);
            file.write(reinterpret_cast<const char *>(data.data()), data.size());
            file.close();

            result = Success;
        }
    }
    return result;
}

Variant PrefabConverter::readJson(const std::string &data, AssetConverterSettings *settings) {
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
        QFile src(settings->source().data());
        if(src.open(QIODevice::WriteOnly)) {
            TString data = Json::save(result, 0);
            src.write(data.data(), data.size());
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

                std::string str = property.toStdString();
                str[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(str[0])));

                propertiesNew[str] = prop.second;
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
