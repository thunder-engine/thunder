#include "assetmanager.h"

#include <QDir>
#include <QMessageBox>

#include "config.h"

#include <json.h>
#include <log.h>

#include <os/uuid.h>

#include "editor/assetconverter.h"
#include "editor/nativecodebuilder.h"
#include "editor/baseassetprovider.h"
#include "editor/projectsettings.h"
#include "editor/pluginmanager.h"

#include "components/actor.h"

#include "systems/resourcesystem.h"

#include "converters/animconverter.h"
#include "converters/textconverter.h"
#include "converters/assimpconverter.h"
#include "converters/fontconverter.h"
#include "converters/prefabconverter.h"
#include "converters/translatorconverter.h"
#include "converters/mapconverter.h"
#include "converters/controlschemeconverter.h"

#define INDEX_VERSION 2

namespace {
    const char *gVersion("version");

    const char *gEntry(".entry");
    const char *gCompany(".company");
    const char *gProject(".project");

    const char *gPersistent("Persistent");
};

AssetManager *AssetManager::m_instance = nullptr;

AssetManager::AssetManager() :
        m_assetProvider(new BaseAssetProvider),
        m_indices(Engine::resourceSystem()->indices()),
        m_projectManager(ProjectSettings::instance()),
        m_timer(new QTimer(this)),
        m_force(false) {

    connect(m_timer, SIGNAL(timeout()), this, SLOT(onPerform()));
}

AssetManager::~AssetManager() {
    std::list<AssetConverter *> list;
    for(auto &it : m_converters) {
        list.push_back(it.second);
    }

    for(AssetConverter *it : std::set<AssetConverter *>(list.begin(), list.end())) {
        delete it;
    }
}

AssetManager *AssetManager::instance() {
    if(!m_instance) {
        m_instance = new AssetManager;
    }
    return m_instance;
}

void AssetManager::destroy() {
    delete m_instance;
    m_instance = nullptr;
}

void AssetManager::init() {
    registerConverter(new AnimConverter);
    registerConverter(new TextConverter);
    registerConverter(new AssimpConverter);
    registerConverter(new FontConverter);
    registerConverter(new PrefabConverter);
    registerConverter(new TranslatorConverter);
    registerConverter(new MapConverter);
    registerConverter(new ControlSchemeConverter);

    for(auto &it : PluginManager::instance()->extensions("converter")) {
        AssetConverter *converter = reinterpret_cast<AssetConverter *>(PluginManager::instance()->getPluginObject(it));
        if(converter) {
            registerConverter(converter);
        }
    }
}

void AssetManager::checkImportSettings(AssetConverterSettings *settings) {
    if(settings->isModified()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText(tr("The import settings has been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        int result = msgBox.exec();
        if(result == QMessageBox::Cancel) {
            return;
        }
        if(result == QMessageBox::Yes) {
            settings->saveSettings();
            pushToImport(settings);
            reimport();
        }
        if(result == QMessageBox::No) {
            settings->loadSettings();
        }
    }
}

void AssetManager::rescan() {
    m_force = false;

    TString target = m_projectManager->targetPath();
    if(target.isEmpty()) {
        m_force |= !Engine::reloadBundle();
        m_force |= m_projectManager->projectSdk() != SDK_VERSION;

        m_assetProvider->init();
    } else {
        m_force = true;
    }

    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/engine/materials").data(),m_force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/engine/textures").data(), m_force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/engine/meshes").data(),   m_force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/engine/pipelines").data(),m_force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/engine/fonts").data(),    m_force);
#ifndef BUILDER
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/editor/materials").data(),m_force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/editor/gizmos").data(),   m_force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/editor/meshes").data(),   m_force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/editor/textures").data(), m_force);
#endif
    m_assetProvider->onDirectoryChangedForce(m_projectManager->contentPath().data(), m_force);

    emit directoryChanged(m_projectManager->contentPath().data());

    reimport();
}

TString AssetManager::assetTypeName(const TString &source) {
    Url url(source);

    TString path = source;

    TString sub;
    if(url.suffix().isEmpty()) {
        path = url.absoluteDir();
        sub = url.name();
    }
    AssetConverterSettings *settings = fetchSettings(path);
    if(settings) {
        if(sub.isEmpty()) {
            return settings->info().type;
        }
        return settings->subItem(sub).type;
    }
    return TString();
}

bool AssetManager::pushToImport(const TString &source) {
    m_assetProvider->onFileChangedForce(source.data(), true);
    return true;
}

bool AssetManager::pushToImport(AssetConverterSettings *settings) {
    if(settings && std::find(m_importQueue.begin(), m_importQueue.end(), settings) == m_importQueue.end()) {
        m_importQueue.push_back(settings);
    }
    return true;
}

void AssetManager::createFromTemplate(const TString &destination) {
    AssetConverter *converter = getConverter(destination);
    if(converter) {
        converter->createFromTemplate(destination);
    } else {
        TString suffix = Url(destination).suffix().toLower();
        for(auto builder : m_builders) {
            for(auto &it : builder->suffixes()) {
                if(it == suffix) {
                    builder->createFromTemplate(destination);
                    return;
                }
            }
        }
    }
}

TString AssetManager::pathToLocal(const TString &source) const {
    static QDir dir(m_projectManager->contentPath().data());
    TString path(dir.relativeFilePath(source.data()).toStdString());
    if(!source.contains(dir.absolutePath().toStdString())) {
        Url info(source);
        path = info.name();
        TString sub;
        if(info.suffix().isEmpty()) {
            path = Url(info.absoluteDir()).name();
            sub = TString("/") + info.name();
        }
        path = TString(".embedded/") + path + sub;
    }
    return path;
}

void AssetManager::reimport() {
    m_importQueue.sort([](AssetConverterSettings *left, AssetConverterSettings *right) {
        return left->type() < right->type();
    });

    emit importStarted(m_importQueue.size(), tr("Importing resources").toStdString());

    m_timer->start(10);
}

void AssetManager::onBuildSuccessful(bool flag, CodeBuilder *builder) {
    for(auto &it : builder->sources()) {
        AssetConverterSettings *settings = fetchSettings(it);
        if(settings) {
            settings->saveSettings();
        }
    }

    emit buildSuccessful(flag);
}

void AssetManager::removeResource(const TString &source) {
    m_assetProvider->removeResource(source);
}

void AssetManager::renameResource(const TString &oldName, const TString &newName) {
    if(oldName != newName) {
        m_assetProvider->renameResource(oldName, newName);
    }
}

void AssetManager::duplicateResource(const TString &source) {
    m_assetProvider->duplicateResource(source);
}

void AssetManager::makePrefab(const TString &source, const TString &target) {
    int index = source.indexOf(':');
    TString id = source.left(index);
    TString name = source.right(index + 1);
    Actor *actor = dynamic_cast<Actor *>(Engine::findObject(id.toLong()));
    if(actor) {
        TString path = m_projectManager->contentPath() + "/" + target + "/" + name + ".fab";

        PrefabConverter *converter = dynamic_cast<PrefabConverter *>(getConverter(path));
        if(converter) {
            Object *parent = actor->parent();

            AssetConverterSettings *settings = converter->createSettings();

            settings->setSource(path);
            settings->info().uuid = Uuid::createUuid().toString();
            m_converterSettings[path] = settings;

            converter->makePrefab(actor, settings);

            registerAsset(settings->source(), settings->info());

            dumpBundle();

            Actor *clone = static_cast<Actor *>(actor->clone(parent));

            emit prefabCreated(id.toLong(), clone->uuid());
        }
    }
}

bool AssetManager::import(const TString &source, const TString &target) {
    TString path;
    if(!Url(target).isAbsolute()) {
        path = m_projectManager->contentPath() + "/";
    }
    path += target + "/";

    Url info(source);

    TString suff = TString(".") + info.suffix();
    TString name = info.baseName();
    findFreeName(name, path, suff);

    return File::copy(source, path + name + suff);
}

AssetConverterSettings *AssetManager::fetchSettings(const TString &source) {
    QDir dir(m_projectManager->contentPath().data());
    TString path((dir.relativeFilePath(source.data())).toStdString());

    auto it = m_converterSettings.find(path);
    if(it != m_converterSettings.end()) {
        return it->second;
    }
    AssetConverterSettings *settings = nullptr;

    if(!path.isEmpty() && File::exists(source)) {
        TString suffix(Url(source).completeSuffix().toLower());
        auto it = m_converters.find(suffix);

        if(it != m_converters.end()) {
            settings = it->second->createSettings();
        } else {
            CodeBuilder *currentBuilder = m_projectManager->currentBuilder();
            CodeBuilder *builder = nullptr;
            for(auto it : m_builders) {
                if(it && (dynamic_cast<NativeCodeBuilder *>(it) == nullptr || it == currentBuilder)) {
                    for(auto &s : it->suffixes()) {
                        if(s == suffix) {
                            builder = it;
                            break;
                        }
                    }
                }
            }

            if(builder) {
                settings = static_cast<AssetConverter *>(builder)->createSettings();
            } else {
                settings = new AssetConverterSettings();
                if(File::isDir(source)) {
                    settings->setDirectory();
                }
            }
        }
        settings->setSource(source);

        if(!settings->loadSettings()) {
            settings->info().uuid = Uuid::createUuid().toString();
        }

        m_converterSettings[path] = settings;
        for(auto &it : settings->subKeys()) {
            m_converterSettings[path + "/" + it] = settings;
        }
    }

    return settings;
}

void AssetManager::registerConverter(AssetConverter *converter) {
    if(converter) {
        CodeBuilder *builder = dynamic_cast<CodeBuilder *>(converter);
        if(builder) {
            m_builders.push_back(builder);
        } else {
            bool valid = false;
            for(TString &format : converter->suffixes()) {
                valid = true;
                m_converters[format.toLower()] = converter;
            }
            if(!valid) {
                delete converter;
                return;
            }
        }
        converter->init();
    }
}

void AssetManager::findFreeName(TString &name, const TString &path, const TString &suff) {
    TString base = name;
    int it = 1;
    while(File::exists(path + "/" + name + suff)) {
        name = base + TString::number(it);
        it++;
    }
}

TString AssetManager::uuidToPath(const TString &uuid) const {
    auto it = m_paths.find(uuid);
    if(it != m_paths.end()) {
        return it->second;
    }
    return TString();
}

TString AssetManager::pathToUuid(const TString &path) const {
    auto it = m_indices.find(path);
    if(it != m_indices.end()) {
        return it->second.uuid;
    }
    it = m_indices.find(pathToLocal(path));
    if(it != m_indices.end()) {
        return it->second.uuid;
    }

    return TString();
}

bool AssetManager::isPersistent(const TString &path) const {
    auto it = m_indices.find(path);
    if(it != m_indices.end()) {
        return (it->second.type == gPersistent);
    }

    return false;
}

QImage AssetManager::icon(const TString &source) {
    AssetConverterSettings *settings = fetchSettings(source);
    if(settings) {
        return settings->icon(pathToUuid(pathToLocal(source)));
    }

    return QImage();
}

Actor *AssetManager::createActor(const TString &source) {
    if(!source.isEmpty()) {
        TString uuid;
        TString path = source;
        if(source.at(0) == '{') {
            uuid = source;
            path = uuidToPath(uuid);
        } else {
            uuid = pathToUuid(source);
        }

        AssetConverterSettings *settings = fetchSettings(path);
        if(settings) {
            AssetConverter *converter = getConverter(path);
            if(converter) {
                return converter->createActor(settings, uuid);
            }
        }
    }
    return nullptr;
}

std::set<TString> AssetManager::labels() const {
    return m_labels;
}

void AssetManager::dumpBundle() {
    VariantMap root;

    VariantMap paths;
    for(auto &it : m_indices) {
        VariantList item;
        item.push_back(it.first);
        item.push_back(it.second.type);
        item.push_back(it.second.md5);
        item.push_back(static_cast<int>(it.second.id));

        paths[it.second.uuid] = item;
    }

    root[gVersion] = INDEX_VERSION;
    root[gContent] = paths;

    VariantMap values;

    values[gEntry] = m_projectManager->firstMap();
    values[gCompany] = m_projectManager->projectCompany();
    values[gProject] = m_projectManager->projectName();

    root[gSettings] = values;

    File file(m_projectManager->importPath() + "/" + gIndex);
    if(file.open(File::WriteOnly)) {
        file.write(Json::save(root, 0));
        file.close();
        Engine::reloadBundle();
    }
}

void AssetManager::onPerform() {
    if(!m_importQueue.empty()) {
        auto settings = m_importQueue.front();
        m_importQueue.pop_front();
        convert(settings);
    } else {
        bool result = false;

        for(CodeBuilder *it : std::as_const(m_builders)) {
            it->rescanSources(m_projectManager->contentPath());
            NativeCodeBuilder *native = dynamic_cast<NativeCodeBuilder *>(it);
            if(!it->isEmpty() && (native == nullptr || (native == m_projectManager->currentBuilder() && m_projectManager->targetPath().isEmpty()))) {
                if(it->isOutdated()) {
                    result = true;

                    if(!it->buildProject()) {
                        m_force = false;
                        m_timer->stop();
                        emit importFinished();
                    }
                }

                TString uuid(it->persistentUUID());
                TString asset(it->persistentAsset());
                if(!uuid.isEmpty() && !asset.isEmpty()) {
                    m_indices[asset] = {gPersistent, uuid};
                    m_paths[uuid] = asset;
                }
            }
        }

        m_assetProvider->cleanupBundle();

        auto tmp = m_indices;
        for(auto &index : tmp) {
            if(!File::exists(m_projectManager->importPath() + "/" + index.second.uuid) && index.second.type != gPersistent) {
                m_indices.erase(m_indices.find(index.first));
            }
        }

        dumpBundle();

        if(result) {
            return;
        }

        m_force = false;
        m_timer->stop();
        emit importFinished();
    }
}

AssetConverter *AssetManager::getConverter(const TString &source) {
    auto it = m_converters.find(Url(source).suffix().toLower());
    if(it != m_converters.end()) {
        return it->second;
    }
    return nullptr;
}

void AssetManager::convert(AssetConverterSettings *settings) {
    AssetConverter *converter = getConverter(settings->source());
    if(converter) {
        settings->setSubItemsDirty();
        uint8_t result = converter->convertFile(settings);
        switch(result) {
            case AssetConverter::Success: {
                aInfo() << "Converting:" << settings->source();

                settings->setCurrentVersion(settings->version());

                TString source = settings->source();
                registerAsset(source, settings->info());

                for(const TString &it : settings->subKeys()) {
                    TString path = source + "/" + it;

                    registerAsset(path, settings->subItem(it));

                    m_converterSettings[pathToLocal(path)] = settings;

                    TString uuid = settings->subItem(it).uuid;
                    if(File::exists(m_projectManager->importPath() + "/" + uuid)) {
                        Engine::reloadResource(uuid);
                        emit imported(path);
                    }
                }

                Engine::reloadResource(settings->destination());

                emit imported(source);

                settings->saveSettings();
            } break;
            default: break;
        }
    } else {
        BuilderSettings *builderSettings = dynamic_cast<BuilderSettings *>(settings);
        if(builderSettings) {
            CodeBuilder *builder = builderSettings->builder();
            if(builder) {
                builder->makeOutdated();
            }
        } else {
            aDebug() << "No Converterter for" << settings->source();
        }
    }
}

StringList AssetManager::templates() const {
    std::set<TString> paths;
    for(auto it : m_builders) {
        TString path(it->templatePath());
        if(!path.isEmpty()) {
            paths.insert(path);
        }
    }

    for(auto &it : m_converters) {
        TString path(it.second->templatePath());
        if(!path.isEmpty()) {
            paths.insert(path);
        }
    }

    return StringList(paths.begin(), paths.end());
}

std::list<CodeBuilder *> AssetManager::builders() const {
    return m_builders;
}

void AssetManager::registerAsset(const TString &source, const ResourceSystem::ResourceInfo &info) {
    if(File::exists(m_projectManager->importPath() + "/" + info.uuid)) {
        TString path = pathToLocal(source);

        m_indices[path] = info;
        m_paths[info.uuid] = source;

        m_labels.insert(info.type);
    }
}

TString AssetManager::unregisterAsset(const TString &source) {
    auto it = m_indices.find(source);
    if(it != m_indices.end()) {
        auto path = m_paths.find(it->second.uuid);
        if(path != m_paths.end() && !path->second.isEmpty()) {
            TString uuid(it->second.uuid);

            m_indices.erase(it);
            m_paths.erase(path);

            return uuid;
        }
    }
    return TString();
}
