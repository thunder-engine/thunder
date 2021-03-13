#include "assetmanager.h"

#include <QCoreApplication>
#include <QDirIterator>
#include <QFileSystemWatcher>
#include <QDir>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaProperty>
#include <QUrl>
#include <QDebug>
#include <QCryptographicHash>

#include <cstring>

#include "config.h"

#include <json.h>
#include <bson.h>

#include <editor/converter.h>
#include <editor/builder.h>

#include <components/scene.h>
#include <components/actor.h>

#include <systems/resourcesystem.h>

#include "animconverter.h"
#include "textconverter.h"
#include "textureconverter.h"
#include "shaderbuilder.h"
#include "assimpconverter.h"
#include "fontconverter.h"
#include "prefabconverter.h"
#include "effectconverter.h"
#include "animationbuilder.h"
#include "translatorconverter.h"
#include "mapconverter.h"

#include "projectmanager.h"
#include "pluginmanager.h"

#include "log.h"

#define BUFF_SIZE 1024

#define INDEX_VERSION 2

#define CODE "Code"

const QString gCRC("crc");
const QString gVersion("version");
const QString gGUID("guid");

const QString gEntry(".entry");
const QString gCompany(".company");
const QString gProject(".project");

AssetManager *AssetManager::m_pInstance = nullptr;

Q_DECLARE_METATYPE(IConverterSettings *)

bool typeLessThan(IConverterSettings *left, IConverterSettings *right) {
    return left->type() < right->type();
}

AssetManager::AssetManager() :
        m_Indices(static_cast<ResourceSystem *>(Engine::resourceSystem())->indices()),
        m_pDirWatcher(new QFileSystemWatcher(this)),
        m_pFileWatcher(new QFileSystemWatcher(this)),
        m_pProjectManager(ProjectManager::instance()),
        m_pTimer(new QTimer(this)),
        m_pEngine(nullptr) {

    m_Icons = {
        {"Invalid",     QImage(":/Style/styles/dark/images/unknown.png")},
        {"Text",        QImage(":/Style/styles/dark/images/text.png")},
        {"Texture",     QImage(":/Style/styles/dark/images/texture.png")},
        {"Material",    QImage(":/Style/styles/dark/images/material.png")},
        {"Mesh",        QImage(":/Style/styles/dark/images/mesh.png")},
        {"Sprite",      QImage(":/Style/styles/dark/images/texture.png")},
        {"Font",        QImage(":/Style/styles/dark/images/ttf.png")},
        {"AnimationClip", QImage(":/Style/styles/dark/images/anim.png")},
        {"ParticleEffect", QImage(":/Style/styles/dark/images/effect.png")},
        {"AudioClip",   QImage(":/Style/styles/dark/images/wav.png")},
        {"Code",        QImage(":/Style/styles/dark/images/cpp.png")},
        {"Map",         QImage(":/Style/styles/dark/images/map.png")},
        {"Pipeline",    QImage(":/Style/styles/dark/images/pipeline.png")},
        {"Prefab",      QImage(":/Style/styles/dark/images/prefab.png")},
        {"AnimationStateMachine", QImage(":/Style/styles/dark/images/actl.png")},
        {"PhysicMaterial", QImage(":/Style/styles/dark/images/fixture.png")},
        {"Translator",  QImage(":/Style/styles/dark/images/l10n.png")},
        {"Pose",        QImage(":/Style/styles/dark/images/pose.png")}
    };

    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(onPerform()));
}

AssetManager::~AssetManager() {
    delete m_pDirWatcher;
    delete m_pFileWatcher;

    for(IConverter *it : QSet<IConverter *>::fromList(m_Converters.values())) {
        delete it;
    }
}

AssetManager *AssetManager::instance() {
    if(!m_pInstance) {
        m_pInstance = new AssetManager;
    }
    return m_pInstance;
}

void AssetManager::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}

void AssetManager::init(Engine *engine) {
    m_pEngine = engine;

    registerConverter(new AnimConverter());
    registerConverter(new AnimationBuilder());
    registerConverter(new TextConverter());
    registerConverter(new TextureConverter());
    registerConverter(new ShaderBuilder());
    registerConverter(new AssimpConverter());
    registerConverter(new FontConverter());
    registerConverter(new PrefabConverter());
    registerConverter(new EffectConverter());
    registerConverter(new TranslatorConverter());
    registerConverter(new MapConverter());

    for(auto it : m_Converters) {
        it->init();
    }
}

void AssetManager::rescan(bool force) {
    QStringList paths = m_pDirWatcher->directories();
    if(!paths.isEmpty()) {
        m_pDirWatcher->removePaths(paths);
    }
    QString target = m_pProjectManager->targetPath();

    QFileInfo info(m_pProjectManager->importPath() + "/" + gIndex);
    m_pEngine->file()->fsearchPathAdd(qPrintable(m_pProjectManager->importPath()), true);

    force |= !target.isEmpty() || !info.exists();

    if(target.isEmpty()) {
        connect(m_pDirWatcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(directoryChanged(QString)));
        connect(m_pDirWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(onDirectoryChanged(QString)));
        connect(m_pDirWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(reimport()));

        connect(m_pFileWatcher, SIGNAL(fileChanged(QString)), this, SIGNAL(fileChanged(QString)));
        connect(m_pFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));
        connect(m_pFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(reimport()));
    }

    onDirectoryChanged(m_pProjectManager->resourcePath() + "/engine/materials",force);
    onDirectoryChanged(m_pProjectManager->resourcePath() + "/engine/textures", force);
    onDirectoryChanged(m_pProjectManager->resourcePath() + "/engine/meshes",   force);
#ifndef BUILDER
    onDirectoryChanged(m_pProjectManager->resourcePath() + "/editor/materials",force);
    onDirectoryChanged(m_pProjectManager->resourcePath() + "/editor/textures", force);
    onDirectoryChanged(m_pProjectManager->resourcePath() + "/editor/meshes",   force);
#endif
    onDirectoryChanged(m_pProjectManager->contentPath(), force);
    emit directoryChanged(m_pProjectManager->contentPath());

    reimport();
}

QString AssetManager::assetTypeName(const QFileInfo &source) {
    QString path = source.filePath();
    QString sub;
    if(source.suffix().isEmpty()) {
        path = source.path();
        sub = source.fileName();
    }
    IConverterSettings *settings = fetchSettings(path);
    if(sub.isEmpty()) {
        return settings->typeName();
    }
    return settings->subTypeName(sub);
}

bool AssetManager::pushToImport(const QFileInfo &source) {
    onFileChanged(source.absoluteFilePath(), true);
    return true;
}

bool AssetManager::pushToImport(IConverterSettings *settings) {
    if(settings) {
        m_ImportQueue.push_back(settings);
    }
    return true;
}

void AssetManager::reimport() {
    std::sort(m_ImportQueue.begin(), m_ImportQueue.end(), typeLessThan);
    emit importStarted(m_ImportQueue.size(), tr("Importing resources"));
    m_pTimer->start(10);
}

bool AssetManager::isOutdated(IConverterSettings *settings) {
    if(settings->version() > settings->currentVersion()) {
        return true;
    }
    bool result = true;

    QFile file(settings->source());
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray md5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        file.close();

        md5 = md5.insert(20, '-');
        md5 = md5.insert(16, '-');
        md5 = md5.insert(12, '-');
        md5 = md5.insert( 8, '-');
        md5.push_front('{');
        md5.push_back('}');

        if(settings->hash() == md5) {
            if(settings->typeName() == CODE || QFileInfo::exists(settings->absoluteDestination())) {
                result = false;
            }
        }
        settings->setHash(md5);
    }
    return result;
}

void AssetManager::removeResource(const QFileInfo &source) {
    QFileInfo src(m_pProjectManager->contentPath() + "/" + source.filePath());
    if(src.isDir()) {
        m_pDirWatcher->removePath(src.absoluteFilePath());

        QDir dir(m_pProjectManager->contentPath());
        QDirIterator it(src.absoluteFilePath(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        while(it.hasNext()) {
            removeResource(QFileInfo(dir.relativeFilePath(it.next())));
        }
        QDir().rmdir(src.absoluteFilePath());
        return;
    } else {
        bool build = false;
        BuilderSettings *settings = dynamic_cast<BuilderSettings *>(fetchSettings(src));
        if(settings) {
            build = true;
        }
        m_pFileWatcher->removePath(src.absoluteFilePath());
        Engine::unloadResource(source.filePath().toStdString());
        string uuid = unregisterAsset(source.filePath().toStdString());
        QFile::remove(m_pProjectManager->importPath() + "/" + uuid.c_str());
        QFile::remove(m_pProjectManager->iconPath() + "/" + uuid.c_str() + ".png");

        QFile::remove(src.absoluteFilePath() + gMetaExt);
        QFile::remove(src.absoluteFilePath());

        if(build) {
            foreach(IBuilder *it, m_Builders) {
                it->rescanSources(ProjectManager::instance()->contentPath());
                if(!it->isEmpty()) {
                    it->convertFile(nullptr);
                    it->buildProject();
                }
            }
        }
    }
    dumpBundle();
}

void AssetManager::renameResource(const QFileInfo &oldName, const QFileInfo &newName) {
    if(oldName.filePath() != newName.filePath()) {
        QFileInfo src(m_pProjectManager->contentPath() + "/" + oldName.filePath());
        QFileInfo dst(m_pProjectManager->contentPath() + "/" + newName.filePath());

        if(src.isDir()) {
            QStringList dirs = m_pDirWatcher->directories();
            QStringList files = m_pFileWatcher->files();
            if(!dirs.isEmpty()) {
                m_pDirWatcher->removePaths(dirs);
                m_pDirWatcher->addPaths(dirs.replaceInStrings(src.absoluteFilePath(), dst.absoluteFilePath()));
            }
            if(!files.isEmpty()) {
                m_pFileWatcher->removePaths(files);
                m_pFileWatcher->addPaths(files.replaceInStrings(src.absoluteFilePath(), dst.absoluteFilePath()));
            }
            QDir dir;
            if(dir.rename(src.absoluteFilePath(), dst.absoluteFilePath())) {
                QMap<QString, QString> back;
                for(auto guid = m_Indices.cbegin(); guid != m_Indices.cend();) {
                    QString path = m_pProjectManager->contentPath() + "/" + guid->first.c_str();
                    if(path.startsWith(src.filePath())) {
                        back[path] = guid->second.second.c_str();
                        guid = m_Indices.erase(guid);
                    } else {
                        ++guid;
                    }
                }

                QMapIterator<QString, QString> it(back);
                while(it.hasNext()) {
                    it.next();
                    QString newPath = it.key();
                    newPath.replace(src.filePath(), dst.filePath());
                    registerAsset(newPath, it.value(), assetTypeName(it.value()));
                }
                dumpBundle();
            } else {
                if(!dirs.isEmpty()) {
                    m_pDirWatcher->addPaths(dirs);
                }
                if(!files.isEmpty()) {
                    m_pFileWatcher->addPaths(files);
                }
            }
        } else {
            if(QFile::rename(src.absoluteFilePath(), dst.absoluteFilePath()) &&
               QFile::rename(src.absoluteFilePath() + gMetaExt, dst.absoluteFilePath() + gMetaExt)) {
                auto it = m_Indices.find(oldName.filePath().toStdString());
                if(it != m_Indices.end()) {
                    QString guid = it->second.second.c_str();
                    m_Indices.erase(it);
                    registerAsset(newName.filePath(), guid, assetTypeName(guid));

                    dumpBundle();
                }
            }
        }

    }
}

namespace
{
// Copied from: https://forum.qt.io/topic/59245/is-there-any-api-to-recursively-copy-a-directory-and-all-it-s-sub-dirs-and-files/3
bool copyRecursively(QString sourceFolder, QString destFolder)
{
    bool success = false;
    QDir sourceDir(sourceFolder);

    if(!sourceDir.exists())
        return false;

    QDir destDir(destFolder);
    if(!destDir.exists())
        destDir.mkdir(destFolder);

    QStringList files = sourceDir.entryList(QDir::Files);
    for(int i = 0; i< files.count(); i++) {
        QString srcName = sourceFolder + QDir::separator() + files[i];
        QString destName = destFolder + QDir::separator() + files[i];
        success = QFile::copy(srcName, destName);
        if(!success)
            return false;
    }

    files.clear();
    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i< files.count(); i++)
    {
        QString srcName = sourceFolder + QDir::separator() + files[i];
        QString destName = destFolder + QDir::separator() + files[i];
        success = copyRecursively(srcName, destName);
        if(!success)
            return false;
    }

    return true;
}
}

void AssetManager::duplicateResource(const QFileInfo &source) {
    QDir dir(m_pProjectManager->contentPath());
    QFileInfo src(m_pProjectManager->contentPath() + "/" + source.filePath());

    QString name = src.baseName();
    QString path = src.absolutePath() + "/";
    QString suff = !src.suffix().isEmpty() ? "." + src.suffix() : "";
    findFreeName(name, path, suff);
    QFileInfo target (src.absoluteFilePath(), path + name + suff);
    if (src.isDir()) {
        copyRecursively(src.absoluteFilePath(), target.absoluteFilePath());
    } else {
        // Source and meta
        QFile::copy(src.absoluteFilePath(), target.filePath());
        QFile::copy(src.absoluteFilePath() + gMetaExt, target.filePath() + gMetaExt);
    }

    IConverterSettings *settings = fetchSettings(target);
    QString guid = settings->destination();
    settings->setDestination(qPrintable(QUuid::createUuid().toString()));
    settings->setAbsoluteDestination(qPrintable(ProjectManager::instance()->importPath() + "/" + settings->destination()));

    settings->saveSettings();

    if(settings->typeName() != CODE) {
        IConverterSettings *s = fetchSettings(src);
        registerAsset(settings->source(), settings->destination(), s->typeName());
    }
    // Icon and resource
    QFile::copy(m_pProjectManager->iconPath() + "/" + guid,
                m_pProjectManager->iconPath() + "/" + settings->destination()+ ".png");

    QFile::copy(m_pProjectManager->importPath() + "/" + guid,
                m_pProjectManager->importPath() + "/" + settings->destination());

    dumpBundle();
}

void AssetManager::makePrefab(const QString &source, const QFileInfo &target) {
    int index = source.indexOf(':');
    QString id = source.left(index);
    QString name = source.mid(index + 1);
    Actor *actor = dynamic_cast<Actor *>(Engine::findObject(id.toUInt(), m_pEngine->scene()));
    if(actor) {
        Actor *clone = static_cast<Actor *>(actor->clone(actor->parent()));
        QString path = target.absoluteFilePath() + "/" + name + ".fab";
        QFile file(path);
        if(file.open(QIODevice::WriteOnly)) {
            string str = Json::save(Engine::toVariant(actor), 0);
            file.write(static_cast<const char *>(&str[0]), str.size());
            file.close();

            IConverterSettings *settings = fetchSettings(path);
            settings->saveSettings();

            Prefab *fab = Engine::objectCreate<Prefab>("");
            fab->setActor(actor);
            clone->setPrefab(fab);

            if(settings->typeName() != CODE) {
                registerAsset(settings->source(), settings->destination(), settings->typeName());

                string dest = settings->destination().toStdString();
                Engine::setResource(fab, dest);
            }
            dumpBundle();

            emit prefabCreated(id.toUInt(), clone->uuid());
        }
    }
}

bool AssetManager::import(const QFileInfo &source, const QFileInfo &target) {
    QString name = source.baseName();
    QString path;
    if(!target.isAbsolute()) {
        path = m_pProjectManager->contentPath() + "/";
    }
    path += target.filePath() + "/";
    QString suff = "." + source.suffix();
    findFreeName(name, path, suff);
    return QFile::copy(source.absoluteFilePath(), path + name + suff);
}

IConverterSettings *AssetManager::fetchSettings(const QFileInfo &source) {
    QDir dir(m_pProjectManager->contentPath());
    QString name = source.fileName();
    QString path = dir.relativeFilePath(source.absoluteFilePath());
    IConverterSettings *settings = m_ConverterSettings.value(path, nullptr);
    if(settings) {
        return settings;
    }

    auto it = m_Converters.find(source.completeSuffix().toLower());
    if(it != m_Converters.end()) {
        settings = it.value()->createSettings();
    } else {
        settings = new IConverterSettings();
    }
    settings->setSource(qPrintable(source.absoluteFilePath()));

    if(!settings->loadSettings()) {
        settings->setDestination( qPrintable(QUuid::createUuid().toString()) );
    }
    settings->setAbsoluteDestination(qPrintable(ProjectManager::instance()->importPath() + "/" + settings->destination()));

    m_ConverterSettings[path] = settings;

    return settings;
}

void AssetManager::registerConverter(IConverter *converter) {
    if(converter) {
        for(QString format : converter->suffixes()) {
            m_Converters[format.toLower()] = converter;

            IBuilder *builder = dynamic_cast<IBuilder *>(converter);
            if(builder) {
                connect(builder, &IBuilder::buildSuccessful, this, &AssetManager::buildSuccessful);

                m_ClassMaps[format.toLower()] = builder->classMap();
                m_Builders.push_back(builder);
            }
        }
    }
}

void AssetManager::findFreeName(QString &name, const QString &path, const QString &suff) {
   QString base = name;
   uint32_t it = 1;
   while(QFileInfo(path + QDir::separator() + name + suff).exists()) {
       name = base + QString::number(it);
       it++;
   }
}

string AssetManager::guidToPath(const string &guid) {
    auto it = m_Paths.find(guid);
    if(it != m_Paths.end()) {
        return it->second.toString();
    }
    return string();
}

string AssetManager::pathToGuid(const string &path) {
    auto it = m_Indices.find(path);
    if(it != m_Indices.end()) {
        return it->second.second;
    }
    it = m_Indices.find(pathToLocal(QString(path.c_str())).toStdString());
    if(it != m_Indices.end()) {
        return it->second.second;
    }
    return string();
}

QImage AssetManager::icon(const QString &source) {
    QImage icon;

    string guid = pathToGuid(source.toStdString()).c_str();

    if(!icon.load(m_pProjectManager->iconPath() + "/" + guid.c_str() + ".png")) {
        QString type = assetTypeName(source);
        icon = m_Icons.value(type);
    }
    return icon;
}

Actor *AssetManager::createActor(const QString &source) {
    if(!source.isEmpty()) {
        QString guid;
        QString path = source;
        if(source[0] == '{') {
            guid = source;
            path = guidToPath(guid.toStdString()).c_str();
        } else {
            guid = pathToGuid(source.toStdString()).c_str();
        }
        QFileInfo info(path);
        IConverter *converter = m_Converters.value(info.suffix().toLower(), nullptr);
        if(converter) {
            return converter->createActor(guid);
        }
    }
    return nullptr;
}

QStringList AssetManager::labels() const {
    return m_Labels;
}

void AssetManager::cleanupBundle() {
    QDirIterator it(m_pProjectManager->importPath(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QFileInfo info(it.next());
        if(!info.isDir() && info.fileName() != gIndex && guidToPath(info.fileName().toStdString()).empty()) {
            QFile::remove(info.absoluteFilePath());
        }
    }

    dumpBundle();
}

void AssetManager::dumpBundle() {
    VariantMap root;

    VariantMap paths;
    for(auto it : m_Indices) {
        VariantList item;
        item.push_back(it.first);
        item.push_back(it.second.first);
        IConverterSettings *settings = fetchSettings(QString(guidToPath(it.second.second).c_str()));
        item.push_back(settings->hash().toStdString());
        paths[it.second.second] = item;
    }

    root[qPrintable(gVersion)] = INDEX_VERSION;
    root[qPrintable(gContent)] = paths;

    VariantMap values;

    values[qPrintable(gEntry)] = qPrintable(m_pProjectManager->firstMap().path);
    values[qPrintable(gCompany)] = qPrintable(m_pProjectManager->projectCompany());
    values[qPrintable(gProject)] = qPrintable(m_pProjectManager->projectName());

    root[qPrintable(gSettings)] = values;

    QFile file(m_pProjectManager->importPath() + "/" + gIndex);
    if(file.open(QIODevice::WriteOnly)) {
         string data = Json::save(root, 0);
        file.write(&data[0], data.size());
        file.close();
        Engine::reloadBundle();
    }
}

void AssetManager::onPerform() {
    QDir dir(m_pProjectManager->contentPath());

    if(!m_ImportQueue.isEmpty()) {
        IConverterSettings *settings = m_ImportQueue.takeFirst();

        if(!convert(settings)) {
            QString dst = m_pProjectManager->importPath() + "/" + settings->destination();
            dir.mkpath(QFileInfo(dst).absoluteDir().absolutePath());
            QFile::copy(settings->source(), dst);
        }
    } else {
        foreach(IBuilder *it, m_Builders) {
            it->rescanSources(ProjectManager::instance()->contentPath());
            if(!it->isEmpty()) {
                QString uuid = it->persistentUUID();
                QString asset = it->persistentAsset();
                m_Indices[asset.toStdString()] = pair<string, string>("", uuid.toStdString());
                m_Paths[uuid.toStdString()] = asset.toStdString();
            }
        }

        cleanupBundle();

        if(isOutdated()) {
            foreach(IBuilder *it, m_Builders) {
                if(!it->isEmpty()) {
                    it->buildProject();
                }
            }
            return;
        }
        m_pTimer->stop();

        m_pDirWatcher->addPath(m_pProjectManager->contentPath());
        m_Labels.removeDuplicates();
        emit importFinished();
    }
}

void AssetManager::onFileChanged(const QString &path, bool force) {
    QDir dir(m_pProjectManager->contentPath());
    QFileInfo info(path);
    if(info.exists() && (QString(".") + info.suffix()) != gMetaExt) {
        IConverterSettings *settings = fetchSettings(info);

        if(force || isOutdated(settings)) {
            pushToImport(settings);
        } else {
            if(settings->typeName() != CODE) {
                QString guid = settings->destination();
                registerAsset(info, guid, settings->typeName());
                for(QString it : settings->subKeys()) {
                    QString value = settings->subItem(it);
                    QString path = info.absoluteFilePath() + "/" + it;
                    registerAsset(path, value, settings->subTypeName(it));
                }
            }
        }
    }
}

void AssetManager::onDirectoryChanged(const QString &path, bool force) {
    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QString item = it.next();
        QFileInfo info(item);
        if((QString(".") + info.suffix()) == gMetaExt || info.isDir()) {
            if(info.isDir()) {
                m_pDirWatcher->addPath(info.absoluteFilePath());
            }
            continue;
        }
        m_pFileWatcher->addPath(info.absoluteFilePath());

        onFileChanged(item, force);
    }
}

IConverter *AssetManager::getConverter(IConverterSettings *settings) {
    QFileInfo info(settings->source());
    QDir dir(m_pProjectManager->contentPath());
    QString format = info.completeSuffix().toLower();

    auto it = m_Converters.find(format);
    if(it != m_Converters.end()) {
        return it.value();
    }
    return nullptr;
}


bool AssetManager::convert(IConverterSettings *settings) {
    QFileInfo info(settings->source());
    QDir dir(m_pProjectManager->contentPath());
    QString format = info.completeSuffix().toLower();

    auto it = m_Converters.find(format);
    if(it != m_Converters.end()) {
        Log(Log::INF) << "Converting:" << qPrintable(settings->source());

        if(it.value()->convertFile(settings) == 0) {
            QString guid = settings->destination();
            QString type = settings->typeName();
            QString source = settings->source();
            registerAsset(source, guid, type);

            for(QString it : settings->subKeys()) {
                QString value = settings->subItem(it);
                QString type = settings->subTypeName(it);
                QString path = source + "/" + it;

                registerAsset(path, value, settings->subTypeName(it));

                if(QFileInfo::exists(m_pProjectManager->importPath() + "/" + value)) {
                    Object *res = Engine::loadResource(value.toStdString());
                    static_cast<ResourceSystem *>(m_pEngine->resourceSystem())->reloadResource(static_cast<Resource *>(res));
                    emit imported(path, type);
                }
            }

            Object *res = Engine::loadResource(guid.toStdString());
            static_cast<ResourceSystem *>(m_pEngine->resourceSystem())->reloadResource(static_cast<Resource *>(res));
            emit imported(source, type);

            settings->saveSettings();

            return true;
        }
    }

    return false;
}

bool AssetManager::isOutdated() const {
    foreach(IBuilder *it, m_Builders) {
        if(it->isOutdated()) {
            return true;
        }
    }
    return false;
}

QString AssetManager::artifact() const {
    return m_Artifact;
}

void AssetManager::setArtifact(const QString &value) {
    m_Artifact = value;
}

AssetManager::ConverterMap AssetManager::converters() const {
    return m_Converters;
}

AssetManager::ClassMap AssetManager::classMaps() const {
    return m_ClassMaps;
}

void AssetManager::registerAsset(const QFileInfo &source, const QString &guid, const QString &type) {
    if(QFileInfo::exists(m_pProjectManager->importPath() + "/" + guid)) {
        QString path = pathToLocal(source);

        m_Indices[path.toStdString()] = pair<string, string>(type.toStdString(), guid.toStdString());
        m_Paths[guid.toStdString()] = source.absoluteFilePath().toStdString();
        m_Labels.push_back(type);
    }
}

string AssetManager::unregisterAsset(const string &source) {
    auto guid = m_Indices.find(source);
    if(guid != m_Indices.end()) {
        string uuid = guid->second.second;
        auto path = m_Paths.find(uuid);
        if(path != m_Paths.end() && !path->second.toString().empty()) {
            m_Indices.erase(guid);
            m_Paths.erase(path);

            return uuid;
        }
    }
    return string();
}

QString AssetManager::pathToLocal(const QFileInfo &source) {
    static QDir dir(m_pProjectManager->contentPath());
    QString path = dir.relativeFilePath(source.absoluteFilePath());
    if(!source.absoluteFilePath().contains(dir.absolutePath())) {
        path = source.fileName();
        QString sub;
        if(source.suffix().isEmpty()) {
            path = QFileInfo(source.path()).fileName();
            sub = QString("/") + source.fileName();
        }
        path = QString(".embedded/") + path + sub;
    }
    return path;
}
