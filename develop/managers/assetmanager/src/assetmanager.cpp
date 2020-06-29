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

#include <zlib.h>
#include <cstring>

#include "config.h"

#include <json.h>
#include <bson.h>

#include "converters/converter.h"
#include "converters/builder.h"

#include <components/scene.h>
#include <components/actor.h>

#include "animconverter.h"
#include "textconverter.h"
#include "textureconverter.h"
#include "shaderbuilder.h"
#include "fbxconverter.h"
#include "fontconverter.h"
#include "prefabconverter.h"
#include "effectconverter.h"
#include "animationbuilder.h"
#include "translatorconverter.h"

#include "projectmanager.h"
#include "pluginmodel.h"

#include "log.h"

#define BUFF_SIZE 1024

const QString gCRC("crc");
const QString gGUID("guid");

const QString gEntry(".entry");
const QString gCompany(".company");
const QString gProject(".project");

AssetManager *AssetManager::m_pInstance   = nullptr;

Q_DECLARE_METATYPE(IConverterSettings *)

AssetManager::AssetManager() :
        m_pEngine(nullptr) {
    m_pProjectManager   = ProjectManager::instance();

    m_pDirWatcher   = new QFileSystemWatcher(this);
    m_pFileWatcher  = new QFileSystemWatcher(this);

    m_pTimer    = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(onPerform()));
}

AssetManager::~AssetManager() {
    foreach(auto it, m_Editors) {
        delete it;
    }
    m_Editors.clear();
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
    m_pEngine   = engine;

    registerConverter(new AnimConverter());
    registerConverter(new AnimationBuilder());
    registerConverter(new TextConverter());
    registerConverter(new TextureConverter());
    registerConverter(new ShaderBuilder());
    registerConverter(new FBXConverter());
    registerConverter(new FontConverter());
    registerConverter(new PrefabConverter());
    registerConverter(new EffectConverter());
    registerConverter(new TranslatorConverter());

    m_Formats["map"]    = IConverter::ContentMap;

    m_ContentTypes[MetaType::type<Mesh *>()] = IConverter::ContentMesh;
    m_ContentTypes[MetaType::type<Pose *>()] = IConverter::ContentPose;
}

void AssetManager::rescan() {
    QStringList paths = m_pDirWatcher->directories();
    if(!paths.isEmpty()) {
        m_pDirWatcher->removePaths(paths);
    }
    QString target  = m_pProjectManager->targetPath();

    QFileInfo info(m_pProjectManager->importPath() + "/" + gIndex);
    m_pEngine->file()->fsearchPathAdd(qPrintable(m_pProjectManager->importPath()), true);

    bool force = !target.isEmpty() || !info.exists();

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
    m_pDirWatcher->addPath(m_pProjectManager->contentPath());
    onDirectoryChanged(m_pProjectManager->contentPath(), force);
    emit directoryChanged(m_pProjectManager->contentPath());

    reimport();
}

void AssetManager::addEditor(uint8_t type, IAssetEditor *editor) {
    m_Editors[type] = editor;
}

QObject *AssetManager::openEditor(const QFileInfo &source) {
    auto it = m_Editors.find(resourceType(source));
    if(it != m_Editors.end()) {
        IAssetEditor *editor    = it.value();
        QDir dir(m_pProjectManager->contentPath());
        editor->loadAsset(createSettings(dir.absoluteFilePath(source.filePath())));
        return dynamic_cast<QObject *>(editor);
    }
    return nullptr;
}

int32_t AssetManager::resourceType(const QFileInfo &source) {
    QString s = source.completeSuffix().toLower();
    auto it = m_Formats.find(s);
    if(it != m_Formats.end()) {
        return it.value();
    }
    return IConverter::ContentInvalid;
}

int32_t AssetManager::assetType(const QString &uuid) {
    auto it = m_Types.find(uuid);
    if(it != m_Types.end()) {
        return it.value();
    }
    return IConverter::ContentInvalid;
}

int32_t AssetManager::toContentType(int32_t type) {
    auto it = m_ContentTypes.find(type);
    if(it != m_ContentTypes.end()) {
        return it.value();
    }
    return type;
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
    emit importStarted(m_ImportQueue.size(), tr("Importing resources"));
    m_pTimer->start(10);
}

bool AssetManager::isOutdated(IConverterSettings *settings) {
    bool result     = true;
    uint32_t crc    = crc32(0L, nullptr, 0);

    QFile file(settings->source());
    if(file.open(QIODevice::ReadOnly)) {
        char buffer[BUFF_SIZE];
        while(!file.atEnd()) {
            memset(buffer, 0, BUFF_SIZE);
            file.read(buffer, BUFF_SIZE);
            crc    = crc32(crc, reinterpret_cast<Bytef *>(buffer), BUFF_SIZE);
        }
        file.close();

        if(settings->crc() == crc) {
            QFileInfo info(settings->absoluteDestination());
            if(settings->type() == IConverter::ContentCode || info.exists()) {
                result  = false;
            }
        }
        settings->setCRC(crc);
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
        m_pFileWatcher->removePath(src.absoluteFilePath());

        auto guid = m_Guids.find(source.filePath().toStdString());
        if(guid != m_Guids.end()) {
            string uuid = guid->second.toString();

            QFile::remove(m_pProjectManager->importPath() + "/" + uuid.c_str());
            QFile::remove(m_pProjectManager->iconPath() + "/" + uuid.c_str() + ".png");

            auto path = m_Paths.find(guid->second.toString());
            if(path != m_Paths.end() && !path->second.toString().empty()) {
                m_Guids.erase(guid);
                m_Paths.erase(path);
                m_Types.remove(uuid.c_str());
            }
        }
        QFile::remove(src.absoluteFilePath() + gMetaExt);
        QFile::remove(src.absoluteFilePath());
    }
    dumpBundle();
}

void AssetManager::renameResource(const QFileInfo &oldName, const QFileInfo &newName) {
    if(oldName.filePath() != newName.filePath()) {
        QFileInfo src(m_pProjectManager->contentPath() + "/" + oldName.filePath());
        QFileInfo dst(m_pProjectManager->contentPath() + "/" + newName.filePath());

        if(src.isDir()) {
            QStringList dirs    = m_pDirWatcher->directories();
            QStringList files   = m_pFileWatcher->files();
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
                for(auto guid = m_Guids.cbegin(); guid != m_Guids.cend();) {
                    QString path    = m_pProjectManager->contentPath() + "/" + guid->first.c_str();
                    if(path.startsWith(src.filePath())) {
                        back[path]  = guid->second.toString().c_str();
                        guid    = m_Guids.erase(guid);
                    } else {
                        ++guid;
                    }
                }

                QDir dir(m_pProjectManager->contentPath());
                QMapIterator<QString, QString> it(back);
                while(it.hasNext()) {
                    it.next();
                    QString newPath = it.key();
                    newPath.replace(src.filePath(), dst.filePath());
                    newPath = dir.relativeFilePath(newPath);
                    string source   = qPrintable(newPath);
                    m_Guids[source] = qPrintable(it.value());
                    m_Paths[qPrintable(it.value())] = source;
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
                auto guid = m_Guids.find(oldName.filePath().toStdString());
                if(guid != m_Guids.end()) {
                    string uuid = guid->second.toString();
                    m_Guids.erase(guid);
                    string source   = newName.filePath().toStdString();
                    m_Guids[source] = uuid;
                    m_Paths[uuid]   = source;
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

    IConverterSettings *settings    = createSettings(target);
    QString guid    = settings->destination();
    settings->setDestination(qPrintable(QUuid::createUuid().toString()));
    settings->setAbsoluteDestination(qPrintable(ProjectManager::instance()->importPath() + "/" + settings->destination()));

    saveSettings(settings);

    if(settings->type() != IConverter::ContentCode) {
        string source   = dir.relativeFilePath(settings->source()).toStdString();
        m_Guids[source] = settings->destination();
        m_Paths[settings->destination()] = source;

        IConverterSettings *s = createSettings(src);
        m_Types[settings->destination()] = m_Types.value(s->destination());
        delete s;
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
        Actor *prefab = static_cast<Actor *>(actor->clone());
        QString path = target.absoluteFilePath() + "/" + name + ".fab";
        QFile file(path);
        if(file.open(QIODevice::WriteOnly)) {
            string str  = Json::save(Engine::toVariant(prefab), 0);
            file.write(static_cast<const char *>(&str[0]), str.size());
            file.close();

            IConverterSettings *settings = createSettings(path);
            saveSettings(settings);

            if(settings->type() != IConverter::ContentCode) {
                QDir dir(m_pProjectManager->contentPath());
                string source   = dir.relativeFilePath(settings->source()).toStdString();
                const char *uuid = settings->destination();
                m_Guids[source] = uuid;
                m_Paths[uuid] = source;
                m_Types[uuid] = settings->type();

                string dest = settings->destination();
                Engine::setResource(prefab, dest);
            }
            dumpBundle();

            actor->setPrefab(prefab);
        }
    }
}

bool AssetManager::import(const QFileInfo &source, const QFileInfo &target) {
    QString name    = source.baseName();
    QString path;
    if(!target.isAbsolute()) {
        path = m_pProjectManager->contentPath() + "/";
    }
    path += target.filePath() + "/";
    QString suff    = "." + source.suffix();
    findFreeName(name, path, suff);
    return QFile::copy(source.absoluteFilePath(), path + name + suff);
}

IConverterSettings *AssetManager::createSettings(const QFileInfo &source) {
    IConverterSettings *settings;
    uint32_t type   = MetaType::INVALID;
    auto it = m_Converters.find(source.completeSuffix().toLower());
    if(it != m_Converters.end()) {
        type = it.value()->contentType();
        settings = it.value()->createSettings();
    } else {
        settings = new IConverterSettings();
        type = resourceType(source);
    }

    settings->setType(type);
    settings->setSource(qPrintable(source.absoluteFilePath()));

    QFile meta(source.absoluteFilePath() + gMetaExt);
    if(meta.open(QIODevice::ReadOnly)) {
        QJsonObject object  = QJsonDocument::fromJson(meta.readAll()).object();
        meta.close();

        QObject *obj = dynamic_cast<QObject *>(settings);
        if(obj) {
            const QMetaObject *meta = obj->metaObject();
            QVariantMap map = object.value(gSettings).toObject().toVariantMap();
            for(int32_t index = 0; index < meta->propertyCount(); index++) {
                QMetaProperty property = meta->property(index);
                QVariant v = map.value(property.name(), property.read(obj));
                v.convert(property.userType());
                property.write(obj, v);

            }
        }
        settings->setDestination( qPrintable(object.value(gGUID).toString()) );
        settings->setCRC( uint32_t(object.value(gCRC).toInt()) );

        QJsonObject sub  = object.value(gSubItems).toObject();
        foreach(QString it, sub.keys()) {
            QJsonArray array = sub.value(it).toArray();
            settings->setSubItem(it, array.first().toString(), array.last().toInt());
        }
    } else {
        settings->setDestination( qPrintable(QUuid::createUuid().toString()) );
    }
    settings->setAbsoluteDestination(qPrintable(ProjectManager::instance()->importPath() + "/" + settings->destination()));

    return settings;
}

void AssetManager::registerConverter(IConverter *converter) {
    if(converter) {
        foreach (QString format, converter->suffixes()) {
            m_Formats[format.toLower()] = converter->contentType();
            m_ContentTypes[converter->type()] = converter->contentType();
            m_Converters[format.toLower()] = converter;

            IBuilder *builder = dynamic_cast<IBuilder *>(converter);
            if(builder) {
                m_pBuilders.push_back(builder);
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
    auto it = m_Guids.find(path);
    if(it != m_Guids.end()) {
        return it->second.toString();
    }
    return string();
}

QImage AssetManager::icon(const QString &path) {
    QImage icon;

    QString guid = pathToGuid(path.toStdString()).c_str();

    if(!icon.load(m_pProjectManager->iconPath() + "/" + guid + ".png", "PNG")) {
        int32_t type = assetType(guid);
        if(type == IConverter::ContentInvalid) {
            type = resourceType(path);
        }

        switch(type) {
            case IConverter::ContentText: {
                icon.load(":/Style/styles/dark/images/text.png", "PNG");
            } break;
            case IConverter::ContentTexture: {
                icon.load(":/Style/styles/dark/images/texture.png", "PNG");
            } break;
            case IConverter::ContentMaterial: {
                icon.load(":/Style/styles/dark/images/material.png", "PNG");
            } break;
            case IConverter::ContentMesh: {
                icon.load(":/Style/styles/dark/images/mesh.png", "PNG");
            } break;
            case IConverter::ContentAtlas: {
                icon.load(":/Style/styles/dark/images/atlas.png", "PNG");
            } break;
            case IConverter::ContentFont: {
                icon.load(":/Style/styles/dark/images/ttf.png", "PNG");
            } break;
            case IConverter::ContentAnimation: {
                icon.load(":/Style/styles/dark/images/anim.png", "PNG");
            } break;
            case IConverter::ContentEffect: {
                icon.load(":/Style/styles/dark/images/effect.png", "PNG");
            } break;
            case IConverter::ContentSound: {
                icon.load(":/Style/styles/dark/images/wav.png", "PNG");
            } break;
            case IConverter::ContentCode: {
                icon.load(":/Style/styles/dark/images/cpp.png", "PNG");
            } break;
            case IConverter::ContentMap: {
                icon.load(":/Style/styles/dark/images/map.png", "PNG");
            } break;
            case IConverter::ContentPipeline: {
                icon.load(":/Style/styles/dark/images/pipeline.png", "PNG");
            } break;
            case IConverter::ContentPrefab: {
                icon.load(":/Style/styles/dark/images/prefab.png", "PNG");
            } break;
            case IConverter::ContentAnimationStateMachine: {
                icon.load(":/Style/styles/dark/images/actl.png", "PNG");
            } break;
            case IConverter::ContentPhysicMaterial: {
                icon.load(":/Style/styles/dark/images/fixture.png", "PNG");
            } break;
            case IConverter::ContentLocalization: {
                icon.load(":/Style/styles/dark/images/l10n.png", "PNG");
            } break;
            case IConverter::ContentPose: {
                icon.load(":/Style/styles/dark/images/pose.png", "PNG");
            } break;
            default: {
                icon.load(":/Style/styles/dark/images/unknown.png", "PNG");
            } break;
        }
    }
    return icon;
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
    QFile file(m_pProjectManager->importPath() + "/" + gIndex);
    if(file.open(QIODevice::WriteOnly)) {
        VariantMap root;
        root[qPrintable(gContent)]  = m_Paths;

        VariantMap settings;

        settings[qPrintable(gEntry)]    = qPrintable(m_pProjectManager->firstMap().path);
        settings[qPrintable(gCompany)]  = qPrintable(m_pProjectManager->projectCompany());
        settings[qPrintable(gProject)]  = qPrintable(m_pProjectManager->projectName());

        root[qPrintable(gSettings)] = settings;
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

            QFileInfo info(settings->source());
            string source = dir.relativeFilePath(info.absoluteFilePath()).toStdString();
            if(!info.absoluteFilePath().contains(dir.absolutePath())) {
                source = string(".embedded/") + info.fileName().toStdString();
            }

            string guid = settings->destination();
            int32_t type = settings->type();
            m_Guids[source] = guid;
            m_Paths[guid] = source;
            m_Types[guid.c_str()] = type;
            for(QString it : settings->subKeys()) {
                string value = settings->subItem(it).toStdString();
                int32_t type = settings->subType(it);
                string path = source + "/" + it.toStdString();
                m_Guids[path] = value;
                m_Paths[value] = path;
                m_Types[value.c_str()] = type;

                emit imported(QString::fromStdString(path), type);
            }
            emit imported(QString::fromStdString(source), type);

            saveSettings(settings);
        }
    } else {
        foreach(IBuilder *it, m_pBuilders) {
            it->rescanSources(ProjectManager::instance()->contentPath());
            if(!it->isEmpty()) {
                QString uuid = it->persistentUUID();
                QString asset = it->persistentAsset();
                m_Paths[uuid.toStdString()] = asset.toStdString();
            }
        }

        cleanupBundle();

        if(isOutdated()) {
            foreach(IBuilder *it, m_pBuilders) {
                if(!it->isEmpty()) {
                    it->buildProject();
                }
            }
            return;
        }
        m_pTimer->stop();

        emit importFinished();
    }
}

void AssetManager::onFileChanged(const QString &path, bool force) {
    QDir dir(m_pProjectManager->contentPath());
    QFileInfo info(path);
    if(info.exists() && (QString(".") + info.suffix()) != gMetaExt) {
        IConverterSettings *settings = createSettings(info);

        //settings->setDestination(settings->source());
        settings->setType(resourceType(info));

        if(force || isOutdated(settings)) {
            pushToImport(settings);
        } else {
            if(settings->type() != IConverter::ContentCode) {
                string source   = dir.relativeFilePath(info.absoluteFilePath()).toStdString();
                if(!info.absoluteFilePath().contains(dir.absolutePath())) {
                    source      = string(".embedded/") + info.fileName().toStdString();
                }
                string guid = settings->destination();
                int32_t type = settings->type();
                m_Guids[source] = guid;
                m_Paths[guid] = source;
                m_Types[guid.c_str()] = type;
                for(QString it : settings->subKeys()) {
                    string value = settings->subItem(it).toStdString();
                    int32_t type = settings->subType(it);
                    string path = source + "/" + it.toStdString();
                    m_Guids[path] = value;
                    m_Paths[value] = path;
                    m_Types[value.c_str()] = type;
                }
            }
        }
    }
}

void AssetManager::onDirectoryChanged(const QString &path, bool force) {
    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QString item    = it.next();
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
    QString format  = info.completeSuffix().toLower();

    auto it = m_Converters.find(format);
    if(it != m_Converters.end()) {
        return it.value();
    }
    return nullptr;
}


bool AssetManager::convert(IConverterSettings *settings) {
    QFileInfo info(settings->source());
    QDir dir(m_pProjectManager->contentPath());
    QString format  = info.completeSuffix().toLower();

    auto it = m_Converters.find(format);
    if(it != m_Converters.end()) {
        Log(Log::INF) << "Converting:" << settings->source();

        settings->setType(it.value()->contentType());
        if(it.value()->convertFile(settings) == 0) {
            string source = dir.relativeFilePath(settings->source()).toStdString();
            if(!info.absoluteFilePath().contains(dir.absolutePath())) {
                source = string(".embedded/") + info.fileName().toStdString();
            }
            string guid = settings->destination();
            int32_t type = settings->type();
            m_Guids[source] = guid;
            m_Paths[guid] = source;
            m_Types[guid.c_str()] = type;

            reloadResource(m_pEngine->loadResource(guid), guid);

            for(QString it : settings->subKeys()) {
                string value = settings->subItem(it).toStdString();
                int32_t type = settings->subType(it);
                string path = source + "/" + it.toStdString();
                m_Guids[path] = value;
                m_Paths[value] = path;
                m_Types[value.c_str()] = type;

                emit imported(QString::fromStdString(path), type);
            }
            emit imported(QString::fromStdString(source), type);

            saveSettings(settings);

            return true;
        }
    }

    return false;
}

void AssetManager::reloadResource(Object *object, const string &path) {
    if(!path.empty()) {
        File *file = Engine::file();
        _FILE *fp = file->_fopen(path.c_str(), "r");
        if(fp) {
            ByteArray data;
            data.resize(file->_fsize(fp));
            file->_fread(&data[0], data.size(), 1, fp);
            file->_fclose(fp);

            Variant var = Bson::load(data);
            if(!var.isValid()) {
                var = Json::load(string(data.begin(), data.end()));
            }
            if(var.isValid()) {
                VariantList objects = var.value<VariantList>();
                VariantList obj = objects.front().value<VariantList>();

                object->loadUserData(obj.back().value<VariantMap>());
            }
        }
    }
}

void AssetManager::saveSettings(IConverterSettings *settings) {
    QJsonObject set;
    QObject *object = dynamic_cast<QObject *>(settings);
    if(object) {
        const QMetaObject *meta = object->metaObject();
        for(int i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty property  = meta->property(i);
            if(QString(property.name()) != "objectName") {
                set.insert(property.name(), QJsonValue::fromVariant(property.read(object)));
            }
        }
    }

    QJsonObject obj;
    obj.insert(gCRC, int(settings->crc()));
    obj.insert(gGUID, settings->destination());
    obj.insert(gSettings, set);
    obj.insert(gType, static_cast<int>(settings->type()));

    QJsonObject sub;
    for(QString it : settings->subKeys()) {
        QJsonArray array;
        array.push_back(settings->subItem(it));
        array.push_back(settings->subType(it));
        sub[it] = array;
    }
    obj.insert(gSubItems, sub);

    QFile fp(QString(settings->source()) + gMetaExt);
    if(fp.open(QIODevice::WriteOnly)) {
        fp.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
        fp.close();
    }
}

bool AssetManager::isOutdated() const {
    foreach(IBuilder *it, m_pBuilders) {
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
