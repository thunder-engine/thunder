#include "converters/prefabconverter.h"

#include "components/actor.h"

#include <QFile>

#include <bson.h>
#include <json.h>

#include <editor/pluginmanager.h>
#include <editor/projectsettings.h>

#define FORMAT_VERSION 2

PrefabConverterSettings::PrefabConverterSettings() {
    setType(MetaType::type<Prefab *>());
    setVersion(FORMAT_VERSION);
}

QStringList PrefabConverterSettings::typeNames() const {
    return { "Prefab" };
}

bool PrefabConverterSettings::isReadOnly() const {
    return false;
}

QString PrefabConverterSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/prefab.svg";
}

AssetConverterSettings *PrefabConverter::createSettings() {
    return new PrefabConverterSettings();
}

QString PrefabConverter::templatePath() const {
    return ":/Templates/Prefab.fab";
}

Actor *PrefabConverter::createActor(const AssetConverterSettings *settings, const QString &guid) const {
    PROFILE_FUNCTION();

    Prefab *prefab = Engine::loadResource<Prefab>(guid.toStdString());
    if(prefab) {
        return static_cast<Actor *>(prefab->actor()->clone());
    }
    return AssetConverter::createActor(settings, guid);
}

AssetConverter::ReturnCode PrefabConverter::convertFile(AssetConverterSettings *settings) {
    PROFILE_FUNCTION();

    AssetConverter::ReturnCode result = InternalError;

    QFile src(settings->source());
    if(src.open(QIODevice::ReadOnly)) {
        std::string data = src.readAll().toStdString();
        src.close();

        Resource *resource = requestResource();

        Variant variant = readJson(data, settings);
        Object *object = Engine::toObject(variant);
        if(object) {
            injectResource(object, resource);

            QFile file(settings->absoluteDestination());
            if(file.open(QIODevice::WriteOnly)) {
                ByteArray data = Bson::save(Engine::toVariant(resource));
                file.write((const char *)&data[0], data.size());
                file.close();

                result = Success;
            }
        }

        delete resource;
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

void PrefabConverter::injectResource(Object *data, Resource *resource) {
    PROFILE_FUNCTION();

    data->setParent(resource);

    Prefab *prefab = dynamic_cast<Prefab *>(resource);
    if(prefab) {
        prefab->setActor(dynamic_cast<Actor *>(data));
    }

    //Object::ObjectList objects;
    //Engine::enumObjects(resource, objects);
    //
    //QSet<QString> modules;
    //for(auto &it : objects) {
    //    QString type = QString::fromStdString(it->typeName());
    //    QString module = PluginManager::instance()->getModuleName(type);
    //    if(!module.isEmpty() && module != (QString("Module") + ProjectSettings::instance()->projectName())) {
    //        modules.insert(module);
    //    }
    //}
    //ProjectSettings::instance()->reportModules(modules);
}

Resource *PrefabConverter::requestResource() {
    return Engine::objectCreate<Prefab>();
}

bool PrefabConverter::toVersion1(Variant &variant) {
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
