#include "assetmanager.h"

#include <QDir>
#include <QUuid>
#include <QMessageBox>

#include "config.h"

#include <json.h>
#include <bson.h>

#include "editor/assetconverter.h"
#include "editor/codebuilder.h"
#include "editor/baseassetprovider.h"

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

#include "editor/projectsettings.h"
#include "editor/pluginmanager.h"

#include "log.h"

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
        m_timer(new QTimer(this)) {

    connect(m_timer, SIGNAL(timeout()), this, SLOT(onPerform()));
}

AssetManager::~AssetManager() {
    std::list<AssetConverter *> list;
    for(auto it : m_converters) {
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
    bool force = false;

    TString target = m_projectManager->targetPath();
    if(target.isEmpty()) {
        force |= !Engine::reloadBundle();
        force |= m_projectManager->projectSdk() != SDK_VERSION;

        m_assetProvider->init();
    } else {
        force = true;
    }

    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/engine/materials").data(),force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/engine/textures").data(), force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/engine/meshes").data(),   force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/engine/pipelines").data(),force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/engine/fonts").data(),    force);
#ifndef BUILDER
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/editor/materials").data(),force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/editor/gizmos").data(),   force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/editor/meshes").data(),   force);
    m_assetProvider->onDirectoryChangedForce((m_projectManager->resourcePath() + "/editor/textures").data(), force);
#endif
    m_assetProvider->onDirectoryChangedForce(m_projectManager->contentPath().data(), force);

    emit directoryChanged(m_projectManager->contentPath().data());

    reimport();
}

TString AssetManager::assetTypeName(const TString &source) {
    QFileInfo info(source.data());

    TString path = info.filePath().toStdString();

    TString sub;
    if(info.suffix().isEmpty()) {
        path = info.path().toStdString();
        sub = info.fileName().toStdString();
    }
    AssetConverterSettings *settings = fetchSettings(path);
    if(settings) {
        if(sub.isEmpty()) {
            return settings->typeName();
        }
        return settings->subTypeName(sub.data());
    }
    return TString();
}

bool AssetManager::pushToImport(const TString &source) {
    m_assetProvider->onFileChangedForce(source.data(), true);
    return true;
}

bool AssetManager::pushToImport(AssetConverterSettings *settings) {
    if(settings) {
        m_importQueue.push_back(settings);
    }
    return true;
}

TString AssetManager::pathToLocal(const TString &source) const {
    static QDir dir(m_projectManager->contentPath().data());
    TString path(dir.relativeFilePath(source.data()).toStdString());
    if(!source.contains(dir.absolutePath().toStdString())) {
        QFileInfo info(source.data());
        path = info.fileName().toStdString();
        TString sub;
        if(info.suffix().isEmpty()) {
            path = QFileInfo(info.path()).fileName().toStdString();
            sub = TString("/") + info.fileName().toStdString();
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

void AssetManager::onBuildSuccessful(CodeBuilder *builder) {
    for(auto &it : builder->sources()) {
        AssetConverterSettings *settings = fetchSettings(it.data());
        if(settings) {
            settings->saveSettings();
        }
    }

    emit buildSuccessful();
}

void AssetManager::removeResource(const TString &source) {
    m_assetProvider->removeResource(source.data());
}

void AssetManager::renameResource(const TString &oldName, const TString &newName) {
    if(oldName != newName) {
        m_assetProvider->renameResource(oldName.data(), newName.data());
    }
}

void AssetManager::duplicateResource(const TString &source) {
    m_assetProvider->duplicateResource(source.data());
}

void AssetManager::makePrefab(const TString &source, const TString &target) {
    int index = source.indexOf(':');
    TString id = source.left(index);
    TString name = source.right(index + 1);
    Actor *actor = dynamic_cast<Actor *>(Engine::findObject(id.toInt()));
    if(actor) {
        TString path(m_projectManager->contentPath());
        path += TString("/") + QFileInfo(target.data()).filePath().toStdString() + "/" + name + ".fab";

        PrefabConverter *converter = dynamic_cast<PrefabConverter *>(getConverter(path));
        if(converter) {
            Object *parent = actor->parent();

            AssetConverterSettings *settings = fetchSettings(path);

            converter->makePrefab(actor, settings);

            registerAsset(settings->source(), settings->destination(), settings->typeName());

            dumpBundle();

            Actor *clone = static_cast<Actor *>(actor->clone(parent));

            emit prefabCreated(id.toInt(), clone->uuid());
        }
    }
}

bool AssetManager::import(const TString &source, const TString &target) {
    QFileInfo info(source.data());
    TString name = info.baseName().toStdString();
    TString path;
    if(!QFileInfo(target.data()).isAbsolute()) {
        path = m_projectManager->contentPath() + "/";
    }
    path += (QFileInfo(target.data()).filePath() + "/").toStdString();
    TString suff = TString(".") + info.suffix().toStdString();
    findFreeName(name, path, suff);

    return QFile::copy(source.data(), (path + name + suff).data());
}

AssetConverterSettings *AssetManager::fetchSettings(const TString &source) {
    QFileInfo info(source.data());

    QDir dir(m_projectManager->contentPath().data());
    TString path((dir.relativeFilePath(info.absoluteFilePath())).toStdString());

    auto it = m_converterSettings.find(path);
    if(it != m_converterSettings.end()) {
        return it->second;
    }
    AssetConverterSettings *settings = nullptr;

    if(!path.isEmpty() && info.exists()) {
        TString suffix(info.completeSuffix().toLower().toStdString());
        auto it = m_converters.find(suffix);

        if(it != m_converters.end()) {
            settings = it->second->createSettings();
        } else {
            CodeBuilder *currentBuilder = m_projectManager->currentBuilder();
            CodeBuilder *builder = nullptr;
            for(auto it : m_builders) {
                if(!it->isNative() || it == currentBuilder) {
                    for(auto s : it->suffixes()) {
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
                if(info.isDir()) {
                    settings->setDirectory();
                }
            }
        }
        settings->setSource(info.absoluteFilePath().toStdString());

        if(!settings->loadSettings()) {
            settings->setDestination(QUuid::createUuid().toString().toStdString());
        }
        if(!settings->isDir()) {
            settings->setAbsoluteDestination(m_projectManager->importPath() + "/" + settings->destination());
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
    while(QFileInfo::exists(QString(path.data()) + QDir::separator() + QString((name + suff).data()))) {
        name = base + TString::number(it);
        it++;
    }
}

TString AssetManager::guidToPath(const TString &guid) const {
    auto it = m_paths.find(guid);
    if(it != m_paths.end()) {
        return it->second.toString();
    }
    return std::string();
}

TString AssetManager::pathToGuid(const TString &path) const {
    auto it = m_indices.find(path);
    if(it != m_indices.end()) {
        return it->second.second;
    }
    it = m_indices.find(pathToLocal(path.data()));
    if(it != m_indices.end()) {
        return it->second.second;
    }

    return std::string();
}

bool AssetManager::isPersistent(const TString &path) const {
    auto it = m_indices.find(path);
    if(it != m_indices.end()) {
        return (it->second.first == gPersistent);
    }

    return false;
}

QImage AssetManager::icon(const TString &source) {
    AssetConverterSettings *settings = fetchSettings(source);
    if(settings) {
        return settings->icon(pathToGuid(pathToLocal(source)).data());
    }

    return QImage();
}

Actor *AssetManager::createActor(const TString &source) {
    if(!source.isEmpty()) {
        TString guid;
        TString path = source;
        if(source.at(0) == '{') {
            guid = source;
            path = guidToPath(guid);
        } else {
            guid = pathToGuid(source);
        }

        AssetConverterSettings *settings = fetchSettings(path);
        if(settings) {
            AssetConverter *converter = getConverter(path);
            if(converter) {
                return converter->createActor(settings, guid);
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
        item.push_back(it.second.first);

        TString path = guidToPath(it.second.second);
        AssetConverterSettings *settings = fetchSettings(path.data());
        if(settings) {
            item.push_back(settings->hash());
            paths[it.second.second] = item;
        } else if(isPersistent(path)) {
            item.push_back("");
            paths[it.second.second] = item;
        }
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
            if(!it->isEmpty()) {
                if(it->isOutdated()) {
                    result = true;

                    if(!it->buildProject()) {
                        m_timer->stop();
                        emit importFinished();
                    }
                }

                TString uuid = it->persistentUUID();
                TString asset = it->persistentAsset();
                if(!uuid.isEmpty() && !asset.isEmpty()) {
                    m_indices[asset] = std::make_pair(gPersistent, uuid);
                    m_paths[uuid] = TString(asset);
                }
            }
        }

        m_assetProvider->cleanupBundle();

        auto tmp = m_indices;
        for(auto &index : tmp) {
            QFileInfo info((m_projectManager->importPath() + "/" + index.second.second).data());
            if(!info.exists() && index.second.first != gPersistent) {
                m_indices.erase(m_indices.find(index.first));
            }
        }

        dumpBundle();

        if(result) {
            return;
        }

        m_timer->stop();
        emit importFinished();
    }
}

AssetConverter *AssetManager::getConverter(const TString &source) {
    auto it = m_converters.find(QFileInfo(source.data()).completeSuffix().toLower().toStdString());
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

                TString guid = settings->destination();
                TString type = settings->typeName();
                TString source = settings->source();
                registerAsset(source, guid, type);

                for(const TString &it : settings->subKeys()) {
                    TString value = settings->subItem(it);
                    TString type = settings->subTypeName(it);
                    TString path = source + "/" + it;

                    registerAsset(path, value, type);

                    m_converterSettings[pathToLocal(path)] = settings;

                    if(QFileInfo::exists((m_projectManager->importPath() + "/" + value).data())) {
                        Engine::reloadResource(value);
                        emit imported(path);
                    }
                }

                Engine::reloadResource(guid);

                emit imported(source);

                settings->saveSettings();
            } break;
            case AssetConverter::CopyAsIs: {
                QDir dir(m_projectManager->contentPath().data());

                TString dst = m_projectManager->importPath() + "/" + settings->destination();
                QFileInfo info(dst.data());
                dir.mkpath(info.absoluteDir().absolutePath());
                QFile::copy(settings->source().data(), dst.data());
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

    for(auto it : m_converters) {
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

void AssetManager::registerAsset(const TString &source, const TString &guid, const TString &type) {
    if(QFileInfo::exists((m_projectManager->importPath() + "/" + guid).data())) {
        TString path = pathToLocal(source);

        m_indices[path] = std::make_pair(type, guid);
        m_paths[guid] = source;

        m_labels.insert(type);
    }
}

TString AssetManager::unregisterAsset(const TString &source) {
    auto guid = m_indices.find(source);
    if(guid != m_indices.end()) {
        TString uuid = guid->second.second;
        auto path = m_paths.find(uuid);
        if(path != m_paths.end() && !path->second.toString().isEmpty()) {
            m_indices.erase(guid);
            m_paths.erase(path);

            return uuid.data();
        }
    }
    return TString();
}
