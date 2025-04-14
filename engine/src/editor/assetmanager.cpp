#include "assetmanager.h"

#include <QDir>
#include <QUuid>
#include <QDebug>
#include <QMessageBox>

#include <QPainter>
#include <QDomDocument>
#include <QTextStream>
#include <QtSvg/QSvgRenderer>

#include <cstring>

#include "config.h"

#include <json.h>
#include <bson.h>

#include "editor/assetconverter.h"
#include "editor/codebuilder.h"
#include "editor/baseassetprovider.h"

#include "components/world.h"
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

Q_DECLARE_METATYPE(AssetConverterSettings *)

AssetManager::AssetManager() :
        m_assetProvider(new BaseAssetProvider),
        m_indices(Engine::resourceSystem()->indices()),
        m_projectManager(ProjectSettings::instance()),
        m_timer(new QTimer(this)),
        m_noIcons(false) {

    connect(m_timer, SIGNAL(timeout()), this, SLOT(onPerform()));
}

AssetManager::~AssetManager() {
    QList<AssetConverter *> list = m_converters.values();
    for(AssetConverter *it : QSet<AssetConverter *>(list.begin(), list.end())) {
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
    m_defaultIcons = {
        {"Invalid", renderDocumentIcon(":/Style/styles/dark/images/unknown.svg") }
    };

    registerConverter(new AnimConverter);
    registerConverter(new TextConverter);
    registerConverter(new AssimpConverter);
    registerConverter(new FontConverter);
    registerConverter(new PrefabConverter);
    registerConverter(new TranslatorConverter);
    registerConverter(new MapConverter);
    registerConverter(new ControlScehemeConverter);

    for(auto &it : PluginManager::instance()->extensions("converter")) {
        AssetConverter *converter = reinterpret_cast<AssetConverter *>(PluginManager::instance()->getPluginObject(it));
        if(converter) {
            registerConverter(converter);
        }
    }
}

void AssetManager::setNoIcons() {
    m_noIcons = true;
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

void AssetManager::rescan(bool force) {
    QString target = m_projectManager->targetPath();

    QFileInfo info(m_projectManager->importPath() + "/" + gIndex);
    Engine::file()->fsearchPathAdd(qPrintable(m_projectManager->importPath()), true);

    force |= !target.isEmpty() || !info.exists();

    if(target.isEmpty()) {
        m_assetProvider->init();
    }

    m_assetProvider->onDirectoryChanged(m_projectManager->resourcePath() + "/engine/materials",force);
    m_assetProvider->onDirectoryChanged(m_projectManager->resourcePath() + "/engine/textures", force);
    m_assetProvider->onDirectoryChanged(m_projectManager->resourcePath() + "/engine/meshes",   force);
    m_assetProvider->onDirectoryChanged(m_projectManager->resourcePath() + "/engine/pipelines",force);
    m_assetProvider->onDirectoryChanged(m_projectManager->resourcePath() + "/engine/fonts",    force);
#ifndef BUILDER
    m_assetProvider->onDirectoryChanged(m_projectManager->resourcePath() + "/editor/materials",force);
    m_assetProvider->onDirectoryChanged(m_projectManager->resourcePath() + "/editor/gizmos",   force);
    m_assetProvider->onDirectoryChanged(m_projectManager->resourcePath() + "/editor/meshes",   force);
    m_assetProvider->onDirectoryChanged(m_projectManager->resourcePath() + "/editor/textures", force);
#endif
    m_assetProvider->onDirectoryChanged(m_projectManager->contentPath(), force);

    emit directoryChanged(m_projectManager->contentPath());

    reimport();
}

QString AssetManager::assetTypeName(const QFileInfo &source) {
    QString path = source.filePath();

    QString sub;
    if(source.suffix().isEmpty()) {
        path = source.path();
        sub = source.fileName();
    }
    AssetConverterSettings *settings = fetchSettings(path);
    if(settings) {
        if(sub.isEmpty()) {
            return settings->typeName();
        }
        return settings->subTypeName(sub);
    }
    return QString();
}

bool AssetManager::pushToImport(const QString &source) {
    m_assetProvider->onFileChanged(source, true);
    return true;
}

bool AssetManager::pushToImport(AssetConverterSettings *settings) {
    if(settings) {
        m_importQueue.push_back(settings);
    }
    return true;
}

void AssetManager::reimport() {
    std::sort(m_importQueue.begin(), m_importQueue.end(), [](AssetConverterSettings *left, AssetConverterSettings *right) {
        return left->type() < right->type();
    });

    emit importStarted(m_importQueue.size(), tr("Importing resources"));

    m_timer->start(10);
}

void AssetManager::onBuildSuccessful() {
    CodeBuilder *builder = dynamic_cast<CodeBuilder *>(sender());
    if(builder) {
        for(auto &it : builder->sources()) {
            AssetConverterSettings *settings = fetchSettings(it);
            if(settings) {
                settings->saveSettings();
            }
        }
    }

    emit buildSuccessful();
}

void AssetManager::removeResource(const QString &source) {
    m_assetProvider->removeResource(source);
}

void AssetManager::renameResource(const QString &oldName, const QString &newName) {
    if(oldName != newName) {
        m_assetProvider->renameResource(oldName, newName);
    }
}

void AssetManager::duplicateResource(const QString &source) {
    m_assetProvider->duplicateResource(source);
}

void AssetManager::makePrefab(const QString &source, const QString &target) {
    int index = source.indexOf(':');
    QString id = source.left(index);
    QString name = source.mid(index + 1);
    Actor *actor = dynamic_cast<Actor *>(Engine::findObject(id.toUInt()));
    if(actor) {
        Object *parent = actor->parent();
        Prefab *fab = Engine::objectCreate<Prefab>("");
        fab->setActor(actor);

        QString path = m_projectManager->contentPath() + "/" + QFileInfo(target).filePath() + "/" + name + ".fab";
        QFile file(path);
        if(file.open(QIODevice::WriteOnly)) {
            std::string str = Json::save(Engine::toVariant(fab), 0);
            file.write(static_cast<const char *>(&str[0]), str.size());
            file.close();

            AssetConverterSettings *settings = fetchSettings(path);
            if(settings) {
                settings->saveSettings();

                if(!settings->isCode()) {
                    registerAsset(settings->source(), settings->destination(), settings->typeName());

                    Engine::setResource(fab, settings->destination().toStdString());
                }
                dumpBundle();

                Actor *clone = static_cast<Actor *>(actor->clone(parent));

                emit prefabCreated(id.toUInt(), clone->uuid());
            }
        }
    }
}

bool AssetManager::import(const QString &source, const QString &target) {
    QFileInfo info(source);
    QString name = info.baseName();
    QString path;
    if(!QFileInfo(target).isAbsolute()) {
        path = m_projectManager->contentPath() + "/";
    }
    path += QFileInfo(target).filePath() + "/";
    QString suff = "." + info.suffix();
    findFreeName(name, path, suff);
    return QFile::copy(source, path + name + suff);
}

AssetConverterSettings *AssetManager::fetchSettings(const QString &source) {
    QFileInfo info(source);

    QDir dir(m_projectManager->contentPath());
    QString path = dir.relativeFilePath(info.absoluteFilePath());

    AssetConverterSettings *settings = m_converterSettings.value(path, nullptr);
    if(settings) {
        return settings;
    }

    if(!path.isEmpty() && info.exists()) {
        QString suffix = info.completeSuffix().toLower();
        auto it = m_converters.find(suffix);

        if(it != m_converters.end()) {
            settings = it.value()->createSettings();
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
            }
        }
        settings->setSource(qPrintable(info.absoluteFilePath()));

        if(!settings->loadSettings()) {
            settings->setDestination( qPrintable(QUuid::createUuid().toString()) );
        }
        settings->setAbsoluteDestination(qPrintable(m_projectManager->importPath() + "/" + settings->destination()));

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
            connect(builder, &CodeBuilder::buildSuccessful, this, &AssetManager::onBuildSuccessful);

            m_builders.push_back(builder);
        } else {
            bool valid = false;
            for(QString &format : converter->suffixes()) {
                valid = true;
                m_converters[format.toLower()] = converter;
            }
            if(!valid) {
                delete converter;
                return;
            }
        }
        converter->init();

        AssetConverterSettings *settings = converter->createSettings();

        for(QString &type : settings->typeNames()) {
            if(!m_defaultIcons.contains(type)) {
                m_defaultIcons[type] = renderDocumentIcon(settings->defaultIcon(type));
            }
        }

        delete settings;
    }
}

void AssetManager::findFreeName(QString &name, const QString &path, const QString &suff) {
    QString base = name;
    uint32_t it = 1;
    while(QFileInfo::exists(path + QDir::separator() + name + suff)) {
        name = base + QString::number(it);
        it++;
    }
}

std::string AssetManager::guidToPath(const std::string &guid) const {
    auto it = m_paths.find(guid);
    if(it != m_paths.end()) {
        return it->second.toString();
    }
    return std::string();
}

std::string AssetManager::pathToGuid(const std::string &path) const {
    auto it = m_indices.find(path);
    if(it != m_indices.end()) {
        return it->second.second;
    }
    it = m_indices.find(pathToLocal(path.c_str()));
    if(it != m_indices.end()) {
        return it->second.second;
    }

    return std::string();
}

bool AssetManager::isPersistent(const std::string &path) const {
    auto it = m_indices.find(path);
    if(it != m_indices.end()) {
        return (it->second.first == gPersistent);
    }

    return false;
}

QImage AssetManager::icon(const QString &source) {
    QImage icon;

    QString guid = pathToGuid(pathToLocal(source)).c_str();

    if(!icon.load(m_projectManager->iconPath() + "/" + guid + ".png")) {
        icon = defaultIcon(source);
    }
    return icon;
}

QImage AssetManager::defaultIcon(const QString &source) {
    return m_defaultIcons.value(assetTypeName(QFileInfo(source)), m_defaultIcons.value("Invalid"));
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

QSet<QString> AssetManager::labels() const {
    return m_labels;
}

void AssetManager::dumpBundle() {
    VariantMap root;

    VariantMap paths;
    for(auto &it : m_indices) {
        VariantList item;
        item.push_back(it.first);
        item.push_back(it.second.first);

        std::string path = guidToPath(it.second.second);
        AssetConverterSettings *settings = fetchSettings(path.c_str());
        if(settings) {
            item.push_back(settings->hash().toStdString());
            paths[it.second.second] = item;
        } else if(isPersistent(path)) {
            item.push_back("");
            paths[it.second.second] = item;
        }
    }

    root[gVersion] = INDEX_VERSION;
    root[qPrintable(gContent)] = paths;

    VariantMap values;

    values[gEntry] = qPrintable(m_projectManager->firstMap().path);
    values[gCompany] = qPrintable(m_projectManager->projectCompany());
    values[gProject] = qPrintable(m_projectManager->projectName());

    root[qPrintable(gSettings)] = values;

    QFile file(m_projectManager->importPath() + "/" + gIndex);
    if(file.open(QIODevice::WriteOnly)) {
        std::string data = Json::save(root, 0);
        file.write(data.data(), data.size());
        file.close();
        Engine::reloadBundle();
    }
}

void AssetManager::onPerform() {
    if(!m_importQueue.isEmpty()) {
        convert(m_importQueue.takeFirst());
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

                QString uuid = it->persistentUUID();
                QString asset = it->persistentAsset();
                if(!uuid.isEmpty() && !asset.isEmpty()) {
                    m_indices[asset.toStdString()] = std::pair<std::string, std::string>(gPersistent, uuid.toStdString());
                    m_paths[uuid.toStdString()] = asset.toStdString();
                }
            }
        }

        m_assetProvider->cleanupBundle();

        auto tmp = m_indices;
        for(auto &index : tmp) {
            QFileInfo info(m_projectManager->importPath() + "/" + index.second.second.c_str());
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

QImage AssetManager::renderDocumentIcon(const QString &path, const QString &color) {
    if(m_noIcons) {
        return QImage();
    }

    QFile file(":/Style/styles/dark/images/document.svg");
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray documentSvg = file.readAll();
        file.close();

        // Add icon
        QDomDocument doc;
        QFile icon(path);
        if(icon.open(QIODevice::ReadOnly)) {
            doc.setContent(&icon);
            icon.close();

            QDomElement svg = doc.firstChildElement("svg");
            QDomElement defs = svg.firstChildElement("defs");
            QDomElement style = defs.firstChildElement("style");

            documentSvg.replace("{style}", qPrintable(style.text()));

            QString str;
            QTextStream stream(&str);

            QDomElement content = defs.nextSiblingElement();
            while(!content.isNull()) {
                content.save(stream, 4);
                content = content.nextSiblingElement();
            }

            documentSvg.replace("{icon}", qPrintable(str));
        }

        documentSvg.replace("{text}", qPrintable(QFileInfo(path).baseName().toLower()));
        documentSvg.replace("#f0f", qPrintable(color));

        QSvgRenderer renderer;
        renderer.load(documentSvg);

        QImage document(128, 128, QImage::Format_ARGB32);
        document.fill(Qt::transparent);

        QPainter painter(&document);
        renderer.render(&painter);

        return document;
    }

    return QImage();
}

AssetConverter *AssetManager::getConverter(const QString &source) {
    auto it = m_converters.find(QFileInfo(source).completeSuffix().toLower());
    if(it != m_converters.end()) {
        return it.value();
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
                aInfo() << "Converting:" << qPrintable(settings->source());

                settings->setCurrentVersion(settings->version());

                QString guid = settings->destination();
                QString type = settings->typeName();
                QString source = settings->source();
                registerAsset(source, guid, type);

                for(const QString &it : settings->subKeys()) {
                    QString value = settings->subItem(it);
                    QString type = settings->subTypeName(it);
                    QString path = source + "/" + it;

                    registerAsset(path, value, type);

                    m_converterSettings[pathToLocal(path).c_str()] = settings;

                    if(QFileInfo::exists(m_projectManager->importPath() + "/" + value)) {
                        Engine::reloadResource(value.toStdString());
                        emit imported(path, type);
                    }
                }

                Engine::reloadResource(guid.toStdString());

                emit imported(source, type);

                settings->saveSettings();
            } break;
            case AssetConverter::CopyAsIs: {
                QDir dir(m_projectManager->contentPath());

                QString dst = m_projectManager->importPath() + "/" + settings->destination();
                QFileInfo info(dst);
                dir.mkpath(info.absoluteDir().absolutePath());
                QFile::copy(settings->source(), dst);
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
            aDebug() << "No Converterter for" << settings->source().toStdString();
        }
    }
}

QStringList AssetManager::templates() const {
    QStringList paths;
    foreach(auto it, m_builders) {
        QString path(it->templatePath());
        if(!path.isEmpty()) {
            paths.push_back(path);
        }
    }

    foreach(auto it, m_converters) {
        QString path(it->templatePath());
        if(!path.isEmpty()) {
            paths.push_back(path);
        }
    }
    paths.removeDuplicates();

    return paths;
}

QList<CodeBuilder *> AssetManager::builders() const {
    return m_builders;
}

void AssetManager::registerAsset(const QString &source, const QString &guid, const QString &type) {
    if(QFileInfo::exists(m_projectManager->importPath() + "/" + guid)) {
        std::string path = pathToLocal(source);

        m_indices[path] = std::pair<std::string, std::string>(type.toStdString(), guid.toStdString());
        m_paths[guid.toStdString()] = source.toStdString();

        m_labels.insert(type);
    }
}

QString AssetManager::unregisterAsset(const QString &source) {
    auto guid = m_indices.find(source.toStdString());
    if(guid != m_indices.end()) {
        std::string uuid = guid->second.second;
        auto path = m_paths.find(uuid);
        if(path != m_paths.end() && !path->second.toString().empty()) {
            m_indices.erase(guid);
            m_paths.erase(path);

            return uuid.c_str();
        }
    }
    return QString();
}

std::string AssetManager::pathToLocal(const QString &source) const {
    static QDir dir(m_projectManager->contentPath());
    QString path(dir.relativeFilePath(source));
    if(!source.contains(dir.absolutePath())) {
        QFileInfo info(source);
        path = info.fileName();
        QString sub;
        if(info.suffix().isEmpty()) {
            path = QFileInfo(info.path()).fileName();
            sub = QString("/") + info.fileName();
        }
        path = QString(".embedded/") + path + sub;
    }
    return path.toStdString();
}
