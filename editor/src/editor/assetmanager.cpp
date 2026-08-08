#include "assetmanager.h"

#include <fstream>

#include "config.h"

#include <json.h>
#include <log.h>
#include <url.h>
#include <file.h>

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

#define VERSION_CHECK(major, minor) ((major<<8)|(minor))

namespace {
    const char *gVersion("version");

    const char *gEntry(".entry");
    const char *gCompany(".company");
    const char *gProjectName(".project");
    const char *gProjectVersion(".version");
};

AssetManager::AssetManager() :
        m_assetProvider(new BaseAssetProvider),
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

void AssetManager::rescan() {
    m_force = false;

    TString target = Editor::project()->targetPath();
    if(target.isEmpty()) {
        bool update = Editor::project()->projectSdk() != SDK_VERSION;
        if(update) {
            getChangedUUIDs();
        }

        Engine::resourceSystem()->unloadBundle(TString());
        m_force |= !Engine::resourceSystem()->loadBundle(TString());
        m_force |= update;
    } else {
        m_force = true;
    }

    m_assetProvider->init(m_force);

    Engine::resourceSystem()->setCleanImport(m_force);

    emit directoryChanged(Editor::project()->contentPath().data());

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
    m_assetProvider->onFileChangedForce(source, true);
    return true;
}

bool AssetManager::pushToImport(AssetConverterSettings *settings) {
    if(settings && std::find(m_importQueue.begin(), m_importQueue.end(), settings) == m_importQueue.end()) {
        m_importQueue.push_back(settings);
    }
    return true;
}

TString AssetManager::pathToLocal(const TString &source) const {
    Url info(source);
    if(!source.contains(Editor::project()->contentPath())) {
        TString path = info.name();
        TString sub;
        if(info.suffix().isEmpty()) {
            path = Url(info.absoluteDir()).name();
            sub = TString("/") + info.name();
        }
        return TString(".embedded/") + path + sub;
    }
    return TString(info.relativeFilePath(Editor::project()->contentPath()));
}

void AssetManager::getChangedUUIDs() {
    uint32_t current = 0;
    StringList currentStr = Editor::project()->projectSdk().split('.');
    if(currentStr.size() >= 2) {
        current = VERSION_CHECK(currentStr.front().toInt(), currentStr.back().toInt());
    }

    StringList targetStr = TString(SDK_VERSION).split('.');
    uint32_t target = VERSION_CHECK(targetStr.front().toInt(), targetStr.back().toInt());

    if(current < target) {
        File file(Editor::project()->resourcePath() + "/uuid.txt");
        if(file.open(File::Read | File::Text)) {
            for(auto &it : Json::load(file.readAll()).toMap()) {
                m_changedUUIDs.push_back(std::make_pair(it.first, it.second.toString()));
            }
            file.close();
        }
    }
}

void AssetManager::fixUUIDs() {
    if(m_changedUUIDs.empty()) {
        return;
    }

    aInfo() << "Fixing dependencies";
    for(auto &path : File::list(Editor::project()->contentPath())) {
        std::ifstream file(path.toStdString(), std::ios::binary | std::ios::ate);
        if(!file) {
            continue;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        char start = 0;
        if(!file.read(&start, 1)) {
            continue;
        }
        if(start != '{' && start != '[' && start != '<') {
            continue;
        }
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        if(!file.read(buffer.data(), size)) {
            continue;
        }
        file.close();

        bool found = false;
        for(auto &it : m_changedUUIDs) {
            for(size_t i = 0; i <= buffer.size() - it.first.size(); ++i) {
                if(std::equal(it.first.toStdString().begin(), it.first.toStdString().end(), buffer.begin() + i)) {
                    std::copy(it.second.toStdString().begin(), it.second.toStdString().end(), buffer.begin() + i);
                    found = true;
                    i += it.first.size() - 1;
                }
            }
        }

        if(!found) {
            continue;
        }

        std::ofstream outFile(path.toStdString(), std::ios::binary | std::ios::trunc);
        if(!outFile) {
            continue;
        }
        outFile.write(buffer.data(), buffer.size());
        pushToImport(fetchSettings(path));
    }
    aInfo() << "Fixed:" << static_cast<int>(m_importQueue.size()) << "files.";
    m_changedUUIDs.clear();
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
        TString path = Editor::project()->contentPath() + "/" + target + "/" + name + ".fab";

        PrefabConverter *converter = dynamic_cast<PrefabConverter *>(getConverter(path));
        if(converter) {
            AssetConverterSettings *settings = converter->createSettings();

            settings->setSource(path);
            settings->newSettings();
            m_converterSettings[path] = settings;

            converter->makePrefab(actor, settings);

            registerAsset(settings->source(), settings->info());

            dumpBundle();

            Object *parent = actor->parent();
            Actor *clone = static_cast<Actor *>(actor->clone(parent));

            emit prefabCreated(id.toLong(), clone->uuid());
        }
    }
}

bool AssetManager::import(const TString &source, const TString &target) {
    TString path;
    if(!Url(target).isAbsolute()) {
        path = Editor::project()->contentPath() + "/";
    }
    path += target + "/";

    Url info(source);

    TString suff = TString(".") + info.suffix();
    TString name = info.baseName();
    findFreeName(name, path, suff);

    return File::copy(source, path + name + suff);
}

AssetConverterSettings *AssetManager::fetchSettings(const TString &source) {
    if(source.isEmpty()) {
        return nullptr;
    }
    TString path(pathToLocal(source));

    auto it = m_converterSettings.find(path);
    if(it != m_converterSettings.end()) {
        return it->second;
    }
    AssetConverterSettings *settings = nullptr;

    if(!path.isEmpty() && File::exists(source)) {
        AssetConverter *converter = getConverter(source);
        if(converter) {
            settings = converter->createSettings();
            settings->setConverter(converter);
        } else {
            TString suffix(Url(source).suffix().toLower());
            CodeBuilder *currentBuilder = Editor::project()->currentBuilder();
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
            settings->newSettings();
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
    ResourceSystem::Aliases &aliases = Engine::resourceSystem()->aliases();
    auto it = aliases.find(path);
    if(it != aliases.end()) {
        return it->second;
    }
    it = aliases.find(pathToLocal(path));
    if(it != aliases.end()) {
        return it->second;
    }

    return TString();
}

Actor *AssetManager::createActor(const TString &source) {
    if(!source.isEmpty()) {
        TString uuid;
        TString path = source;
        if(source.front() == '{') {
            uuid = source;
            path = uuidToPath(uuid);
        } else {
            uuid = pathToUuid(source);
        }

        AssetConverterSettings *settings = fetchSettings(path);
        if(settings) {
            AssetConverter *converter = settings->converter();
            if(converter) {
                return converter->createActor(settings, uuid);
            }
        }
    }
    return nullptr;
}

StringList AssetManager::labels() const {
    return StringList(m_labels.begin(), m_labels.end());
}

void AssetManager::dumpBundle() {
    VariantMap root;

    VariantMap paths;
    for(auto &it : Engine::resourceSystem()->indices()) {
        VariantList item;
        item.push_back(pathToLocal(uuidToPath(it.first)));
        item.push_back(it.second.type);
        item.push_back(it.second.md5);
        item.push_back(static_cast<int>(it.second.id));

        paths[it.second.uuid] = item;
    }

    root[gVersion] = INDEX_VERSION;
    root[gContent] = paths;

    VariantMap values;

    values[gEntry] = Editor::project()->firstMap();
    values[gCompany] = Editor::project()->projectCompany();
    values[gProjectName] = Editor::project()->projectName();
    values[gProjectVersion] = Editor::project()->projectVersion();

    root[gSettings] = values;

    File file(Editor::project()->importPath() + "/" + gIndex);
    if(file.open(File::Write)) {
        file.write(Json::save(root, 0));
        file.close();

        Engine::resourceSystem()->unloadBundle(TString());
        Engine::resourceSystem()->loadBundle(TString());
    }
}

void AssetManager::onPerform() {
    if(!m_importQueue.empty()) {
        auto settings = m_importQueue.front();
        m_importQueue.pop_front();
        convert(settings);
    } else {
        bool result = false;

        fixUUIDs();
        if(!m_importQueue.empty()) {
            return;
        }

        for(CodeBuilder *it : std::as_const(m_builders)) {
            it->rescanSources(Editor::project()->contentPath());
            NativeCodeBuilder *native = dynamic_cast<NativeCodeBuilder *>(it);
            if(!it->isEmpty() && (native == nullptr || (native == Editor::project()->currentBuilder() && Editor::project()->targetPath().isEmpty()))) {
                if(it->isOutdated()) {
                    result = true;

                    if(!it->buildProject()) {
                        m_force = false;
                        m_timer->stop();
                        Engine::resourceSystem()->setCleanImport(m_force);
                        emit importFinished();
                    }
                }
            }
        }

        // Cleanup bundle
        for(auto &path : File::list(Editor::project()->importPath())) {
            TString fileName(Url(path).name());
            if(!File::isDir(path) && fileName != gIndex && uuidToPath(fileName).isEmpty()) {
                File::remove(path);
            }
        }

        ResourceSystem::Dictionary &indices = Engine::resourceSystem()->indices();
        auto tmp = indices;
        for(auto &index : tmp) {
            if(index.second.uuid.isEmpty() || (!File::exists(Editor::project()->importPath() + "/" + index.second.uuid))) {
                indices.erase(index.second.uuid);
            }
        }

        dumpBundle();

        if(result) {
            return;
        }

        m_force = false;
        m_timer->stop();
        Engine::resourceSystem()->setCleanImport(m_force);
        emit importFinished();
    }
}

AssetConverter *AssetManager::getConverter(const TString &source) {
    auto it = m_converters.find(Url(source).completeSuffix().toLower());
    if(it != m_converters.end()) {
        return it->second;
    }
    return nullptr;
}

void AssetManager::convert(AssetConverterSettings *settings) {
    TString source(settings->source());
    AssetConverter *converter = getConverter(source);
    if(converter) {
        settings->setSubItemsDirty();
        uint8_t result = converter->convertFile(settings);
        switch(result) {
            case AssetConverter::Success: {
                aInfo() << "Converting:" << source;
                settings->setCurrentVersion(settings->version());

                registerAsset(source, settings->info());

                for(const TString &it : settings->subKeys()) {
                    TString path = source + "/" + it;
                    registerAsset(path, settings->subItem(it));
                    m_converterSettings[pathToLocal(path)] = settings;

                    TString uuid = settings->subItem(it).uuid;
                    if(File::exists(Editor::project()->importPath() + "/" + uuid)) {
                        Engine::reloadResource(uuid);
                        emit imported();
                    }
                }

                Engine::reloadResource(settings->destination());
                emit imported();

                settings->saveSettings();
                auto &list = settings->changedUuids();
                m_changedUUIDs.insert(m_changedUUIDs.end(), list.begin(), list.end());
                settings->clearChangedUuids();
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
            aDebug() << "No Converterter for" << source;
        }
    }
}

std::list<AssetConverter *> AssetManager::converters() const {
    std::set<AssetConverter *> result;
    for(auto &it : m_converters) {
        result.insert(it.second);
    }

    return std::list<AssetConverter *>(result.begin(), result.end());
}

std::list<CodeBuilder *> AssetManager::builders() const {
    return m_builders;
}

void AssetManager::registerAsset(const TString &source, const ResourceSystem::ResourceInfo &info) {
    if(File::exists(Editor::project()->importPath() + "/" + info.uuid)) {
        TString path = pathToLocal(source);

        ResourceSystem::Dictionary &indices = Engine::resourceSystem()->indices();
        indices[info.uuid] = info;
        ResourceSystem::Aliases &aliases = Engine::resourceSystem()->aliases();
        aliases[path] = info.uuid;

        m_paths[info.uuid] = source;

        m_labels.insert(info.type);
    }
}

TString AssetManager::unregisterAsset(const TString &source) {
    TString uuid(pathToUuid(source));
    if(!uuid.isEmpty()) {
        ResourceSystem::Dictionary &indices = Engine::resourceSystem()->indices();
        indices.erase(uuid);
        ResourceSystem::Aliases &aliases = Engine::resourceSystem()->aliases();
        aliases.erase(pathToLocal(source));
        m_paths.erase(uuid);

        return uuid;
    }

    return TString();
}
