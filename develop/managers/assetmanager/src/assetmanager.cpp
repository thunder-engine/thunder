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

#include "converters/converter.h"

#include "components/scene.h"
#include "components/actor.h"

#include "animconverter.h"
#include "textconverter.h"
#include "textureconverter.h"
#include "materialconverter.h"
#include "fbxconverter.h"
#include "fontconverter.h"
#include "prefabconverter.h"
#include "effectconverter.h"

#include "projectmanager.h"
#include "pluginmodel.h"

#include "qbsbuilder.h"

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
        m_pEngine(nullptr),
        m_Outdated(false) {
    m_pProjectManager   = ProjectManager::instance();

    m_pDirWatcher   = new QFileSystemWatcher(this);
    m_pFileWatcher  = new QFileSystemWatcher(this);
    connect(m_pDirWatcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(directoryChanged(QString)));
    connect(m_pDirWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(onDirectoryChanged(QString)));
    connect(m_pDirWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(reimport()));

    connect(m_pFileWatcher, SIGNAL(fileChanged(QString)), this, SIGNAL(fileChanged(QString)));
    connect(m_pFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(onFileChanged(QString)));
    connect(m_pFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(reimport()));

    m_pTimer    = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(onPerform()));

    registerConverter(new AnimConverter());
    registerConverter(new TextConverter());
    registerConverter(new TextureConverter());
    registerConverter(new MaterialConverter());
    registerConverter(new FBXConverter());
    registerConverter(new FontConverter());
    registerConverter(new PrefabConverter());
    registerConverter(new EffectConverter());

    QbsBuilder *builder = new QbsBuilder();
    builder->setEnvironment(QStringList(), QStringList(), QStringList());
    m_pBuilder  = builder;

    connect(m_pBuilder, SIGNAL(buildFinished(int)), this, SLOT(onBuildFinished(int)));

    m_Formats["map"]    = IConverter::ContentMap;
    m_Formats["cpp"]    = IConverter::ContentCode;
    m_Formats["h"]      = IConverter::ContentCode;
}

AssetManager::~AssetManager() {
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
    QString target  = m_pProjectManager->targetPath();

    onDirectoryChanged(m_pProjectManager->resourcePath() + "/engine/shaders",  !target.isEmpty());
    onDirectoryChanged(m_pProjectManager->resourcePath() + "/engine/materials",!target.isEmpty());
    onDirectoryChanged(m_pProjectManager->resourcePath() + "/engine/textures", !target.isEmpty());
    onDirectoryChanged(m_pProjectManager->resourcePath() + "/engine/meshes",   !target.isEmpty());
#ifndef BUILDER
    onDirectoryChanged(m_pProjectManager->resourcePath() + "/editor/meshes",   !target.isEmpty());
    onDirectoryChanged(m_pProjectManager->resourcePath() + "/editor/materials",!target.isEmpty());
    onDirectoryChanged(m_pProjectManager->resourcePath() + "/editor/textures", !target.isEmpty());
#endif
    m_pDirWatcher->addPath(m_pProjectManager->contentPath());
    onDirectoryChanged(m_pProjectManager->contentPath(), !target.isEmpty());

    reimport();
    cleanupBundle();
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
    return MetaType::INVALID;
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
    if(!m_ImportQueue.isEmpty()) {
        emit importStarted(m_ImportQueue.size(), tr("Importing resources"));
        m_pTimer->start(10);
    } else {
        emit importFinished();
    }
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
            if(info.exists()) {
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

void AssetManager::duplicateResource(const QFileInfo &source) {
    QDir dir(m_pProjectManager->contentPath());
    QFileInfo src(m_pProjectManager->contentPath() + "/" + source.filePath());

    QString name = src.baseName();
    QString path = src.absolutePath() + "/";
    QString suff = "." + src.suffix();
    findFreeName(name, path, suff);

    QFileInfo target(src.absoluteFilePath(), path + name + suff);
    // Source and meta
    QFile::copy(src.absoluteFilePath(), target.filePath());
    QFile::copy(src.absoluteFilePath() + gMetaExt, target.filePath() + gMetaExt);

    IConverterSettings *settings    = createSettings(target);
    QString guid    = settings->destination();
    settings->setDestination(qPrintable(QUuid::createUuid().toString()));
    settings->setAbsoluteDestination(qPrintable(ProjectManager::instance()->importPath() + "/" + settings->destination()));

    saveSettings(settings);

    if(settings->type() != IConverter::ContentCode) {
        string source   = dir.relativeFilePath(settings->source()).toStdString();
        m_Guids[source] = settings->destination();
        m_Paths[settings->destination()]    = source;
    }
    // Icon and resource
    QFile::copy(m_pProjectManager->iconPath() + "/" + guid,
                m_pProjectManager->iconPath() + "/" + settings->destination()+ ".png");

    QFile::copy(m_pProjectManager->importPath() + "/" + guid,
                m_pProjectManager->importPath() + "/" + settings->destination());

    dumpBundle();
}

void AssetManager::makePrefab(const QString &source, const QFileInfo &target) {
    Actor *actor  = dynamic_cast<Actor *>(m_pEngine->scene()->find(source.toStdString()));
    if(actor) {
        Actor *prefab   = static_cast<Actor *>(actor->clone());
        QString path    = target.absoluteFilePath() + "/" + QUrl(source).fileName() + ".fab";
        QFile file(path);
        if(file.open(QIODevice::WriteOnly)) {
            string str  = Json::save(Engine::toVariant(prefab), 0);
            file.write(static_cast<const char *>(&str[0]), str.size());
            file.close();

            IConverterSettings *settings    = createSettings(path);
            saveSettings(settings);

            if(settings->type() != IConverter::ContentCode) {
                QDir dir(m_pProjectManager->contentPath());
                string source   = dir.relativeFilePath(settings->source()).toStdString();
                m_Guids[source] = settings->destination();
                m_Paths[settings->destination()]    = source;

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
        path    = m_pProjectManager->contentPath() + "/";
    }
    path       += target.filePath() + "/";
    QString suff    = "." + source.suffix();
    findFreeName(name, path, suff);
    return QFile::copy(source.absoluteFilePath(), path + name + suff);
}

IConverterSettings *AssetManager::createSettings(const QFileInfo &source) {
    IConverterSettings *settings;
    uint32_t type   = MetaType::INVALID;
    auto it = m_Converters.find(source.completeSuffix().toLower());
    if(it != m_Converters.end()) {
        type    = it.value()->type();
        settings    = it.value()->createSettings();
    } else {
        settings    = new IConverterSettings();
        type = resourceType(source);
    }

    settings->setType(type);
    settings->setSource(qPrintable(source.absoluteFilePath()));

    QFile meta(source.absoluteFilePath() + gMetaExt);
    if(meta.open(QIODevice::ReadOnly)) {
        QJsonObject object  = QJsonDocument::fromJson(meta.readAll()).object();
        meta.close();

        settings->loadProperties( object.value(gSettings).toObject().toVariantMap() );
        settings->setDestination( qPrintable(object.value(gGUID).toString()) );
        settings->setCRC( uint32_t(object.value(gCRC).toInt()) );

        QJsonArray array    = object.value(gSubItems).toArray();
        foreach(QJsonValue it, array) {
            settings->addSubItem(qPrintable(it.toString()));
        }
    } else {
        settings->setDestination( qPrintable(QUuid::createUuid().toString()) );
    }
    if(settings->type() == IConverter::ContentCode) {
        settings->setAbsoluteDestination(qPrintable(artifact()));
    } else {
        settings->setAbsoluteDestination(qPrintable(ProjectManager::instance()->importPath() + "/" + settings->destination()));
    }
    return settings;
}

void AssetManager::registerConverter(IConverter *converter) {
    if(converter) {
        foreach (QString format, QString::fromStdString(converter->format()).split(';')) {
            m_Formats[format.toLower()]     = converter->contentType();
            m_ContentTypes[converter->type()]   = converter->contentType();
            m_Converters[format.toLower()]  = converter;
        }
    }
}

void AssetManager::findFreeName(QString &name, const QString &path, const QString &suff) {
   QString base    = name;
   uint32_t it     = 1;
   while(QFileInfo(path + QDir::separator() + name + suff).exists()) {
       name        = base + QString::number(it);
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
    switch(resourceType(path)) {
        case IConverter::ContentMap: {
            icon.load(":/Style/styles/dark/images/map.png", "PNG");
        } break;
        case IConverter::ContentCode: {
            icon.load(":/Style/styles/dark/images/cpp.png", "PNG");
        } break;
        case IConverter::ContentFont: {
            icon.load(":/Style/styles/dark/images/ttf.png", "PNG");
        } break;
        case IConverter::ContentAnimation: {
            icon.load(":/Style/styles/dark/images/anim.png", "PNG");
        } break;
        case IConverter::ContentSound: {
            icon.load(":/Style/styles/dark/images/wav.png", "PNG");
        } break;
        case IConverter::ContentPrefab: {
            icon.load(":/Style/styles/dark/images/prefab.png", "PNG");
        } break;
        default: {
            QStringList list;
            for(auto it : m_Guids) {
                list << QString::fromStdString( it.first );
            }
            icon.load(m_pProjectManager->iconPath() + "/" + pathToGuid(path.toStdString()).c_str() + ".png", "PNG");
        } break;
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
            if(settings->type() == IConverter::ContentCode) {
                setOutdated();
            } else {
                QString dst = m_pProjectManager->importPath() + "/" + settings->destination();
                dir.mkpath(QFileInfo(dst).absoluteDir().absolutePath());
                QFile::copy(settings->source(), dst);

                QFileInfo info(settings->source());
                string source   = dir.relativeFilePath(info.absoluteFilePath()).toStdString();
                if(!info.absoluteFilePath().contains(dir.absolutePath())) {
                    source      = string(".embedded/") + info.fileName().toStdString();
                }

                m_Guids[source] = settings->destination();
                m_Paths[settings->destination()]    = source;
                for(uint32_t i = 0; i < settings->subItemsCount(); i++) {
                    m_Paths[settings->subItem(i)]   = source;
                }
                emit imported(QString::fromStdString(m_Paths[settings->destination()].toString()), toContentType(settings->type()));
            }
            saveSettings(settings);
        }
    } else {
        dumpBundle();
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
                string guid     = settings->destination();
                m_Guids[source] = guid;
                m_Paths[guid]   = source;
                for(uint32_t i = 0; i < settings->subItemsCount(); i++) {
                    m_Paths[settings->subItem(i)]   = source;
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

bool AssetManager::convert(IConverterSettings *settings) {
    QFileInfo info(settings->source());
    QDir dir(m_pProjectManager->contentPath());
    QString format  = info.completeSuffix().toLower();

    auto it = m_Converters.find(format);
    if(it != m_Converters.end()) {
        Log(Log::INF) << "Converting:" << settings->source();

        settings->setType(it.value()->type());
        if(it.value()->convertFile(settings) == 0) {
            string source   = dir.relativeFilePath(settings->source()).toStdString();
            if(!info.absoluteFilePath().contains(dir.absolutePath())) {
                source      = string(".embedded/") + info.fileName().toStdString();
            }
            m_Guids[source] = settings->destination();
            m_Paths[settings->destination()]    = source;
            for(uint32_t i = 0; i < settings->subItemsCount(); i++) {
                m_Paths[settings->subItem(i)]   = source;
            }
            saveSettings(settings);

            emit imported(QString::fromStdString(source), toContentType(settings->type()));

            return true;
        }
    }

    return false;
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

    QJsonArray subItems;
    for(uint32_t i = 0; i < settings->subItemsCount(); i++) {
        subItems.append(settings->subItem(i));
    }
    obj.insert(gSubItems, subItems);

    QFile fp(QString(settings->source()) + gMetaExt);
    if(fp.open(QIODevice::WriteOnly)) {
        fp.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
        fp.close();
    }
}

void AssetManager::rebuildProject() {
    setOutdated();
    buildProject();
}

void AssetManager::buildProject() {
    if(m_Outdated) {
        m_pBuilder->buildProject();
    }
}

void AssetManager::onBuildFinished(int exitCode) {
    if(exitCode == 0) {
        PluginModel::instance()->reloadPlugin(artifact());
        m_Outdated  = false;
    }
    emit buildFinished(exitCode);
}

bool AssetManager::isOutdated() const {
    return m_Outdated;
}

void AssetManager::setOutdated() {
    m_Outdated  = true;
}

QString AssetManager::artifact() const {
    return m_pBuilder->artifact();
}
