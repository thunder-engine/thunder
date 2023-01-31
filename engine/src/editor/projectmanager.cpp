#include "projectmanager.h"

#include <QUuid>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>

#include <QMetaProperty>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QSettings>

#include <log.h>

#include "config.h"

#include <editor/pluginmanager.h>

namespace {
    const char *gProject("ProjectId");
    const char *gProjects("Projects");
    const char *gProjectSdk("ProjectSdk");

    const char *gModulesFile("/modules.txt");
};

ProjectManager *ProjectManager::m_pInstance = nullptr;

ProjectManager::ProjectManager() {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#if defined(Q_OS_MAC)
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#endif

    m_sdkPath = QFileInfo(dir.absolutePath());
    m_resourcePath = QFileInfo(sdkPath() + "/resources");
    m_templatePath = QFileInfo(resourcePath() + "/editor/templates");

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QString path = settings.value(gProjects, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    m_myProjectsPath = QFileInfo(path);
}

ProjectManager *ProjectManager::instance() {
    if(!m_pInstance) {
        m_pInstance = new ProjectManager;
    }
    return m_pInstance;
}

void ProjectManager::destroy() {
    delete m_pInstance;
    m_pInstance = nullptr;
}

void ProjectManager::init(const QString &project, const QString &target) {
    m_projectPath = QFileInfo(project);

    for(auto &it : PluginManager::instance()->extensions("converter")) {
        AssetConverter *converter = reinterpret_cast<AssetConverter *>(PluginManager::instance()->getPluginObject(it));
        CodeBuilder *builder = dynamic_cast<CodeBuilder *>(converter);
        if(builder) {
            for(auto &platform : builder->platforms()) {
                m_supportedPlatforms[platform] = builder;
            }
        }
    }

    if(!target.isEmpty()) {
        QDir dir;
        dir.mkpath(target);
    }
    m_targetPath = QFileInfo(target);

    loadSettings();

    m_projectName = m_projectPath.completeBaseName();

    m_contentPath = QFileInfo(m_projectPath.absolutePath() + QDir::separator() + gContent);
    m_pluginsPath = QFileInfo(m_projectPath.absolutePath() + QDir::separator() + gPlugins);
    m_cachePath = QFileInfo(m_projectPath.absolutePath() + QDir::separator() + gCache);

    m_iconPath = QFileInfo(m_cachePath.absoluteFilePath() + QDir::separator() + gThumbnails);
    m_generatedPath = QFileInfo(m_cachePath.absoluteFilePath() + QDir::separator() + gGenerated);

    m_manifestFile = QFileInfo(m_projectPath.absolutePath() + QDir::separator() + gPlatforms + "/android/AndroidManifest.xml");

    QDir dir;
    dir.mkpath(m_contentPath.absoluteFilePath());
    dir.mkpath(m_iconPath.absoluteFilePath());
    dir.mkpath(m_generatedPath.absoluteFilePath());
    dir.mkpath(m_pluginsPath.absoluteFilePath());

    QFile file(m_generatedPath.absolutePath() + gModulesFile);
    if(file.open(QIODevice::ReadOnly)) {
        for(auto &it : file.readAll().split('\n')) {
            m_autoModules += it;
        }
        file.close();
    }

    setCurrentPlatform();
}

void ProjectManager::loadSettings() {
    QFile file(m_projectPath.absoluteFilePath());
    if(file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        m_projectId = QUuid::createUuid().toString();
        if(!doc.isNull()) {
            const QMetaObject *meta = metaObject();
            QJsonObject object = doc.object();
            foreach(const QString &it, object.keys()) {
                QVariant value = object.value(it).toVariant();

                int index = meta->indexOfProperty(qPrintable(it));
                if(index > -1) {
                    QMetaProperty property = meta->property(index);
                    if(property.userType() == qMetaTypeId<Template>()) {
                        value = QVariant::fromValue<Template>(Template(value.toString(), MetaType::type<Actor *>()));
                    }
                    property.write(this, value);
                } else {
                    setProperty(qPrintable(it), value); // Dynamic property because some module may store this
                }
            }
            {
                QJsonObject::iterator it = object.find(gProjectSdk);
                if(it != doc.object().end()) {
                    m_projectSdk = it.value().toString();
                }
            }
            {
                QJsonObject::iterator it = object.find(gProject);
                if(it != doc.object().end()) {
                    m_projectId = it.value().toString();
                }
            }
            {
                QJsonObject::iterator it = object.find(gPlatforms);
                if(it != doc.object().end()) {
                    foreach(auto platform, it.value().toArray()) {
                        m_platforms << platform.toString();
                    }
                }
            }
            {
                QJsonObject::iterator it = object.find(gModules);
                if(it != doc.object().end()) {
                    foreach(auto module, it.value().toArray()) {
                        m_modules << module.toString();
                    }
                }
            }
        }
        m_autoModules.insert("RenderGL");

        saveSettings();
    }
}

void ProjectManager::saveSettings() {
    QJsonDocument doc;

    const QMetaObject *meta = metaObject();

    QJsonObject object;

    bool success = true;
    QStringList req;
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property = meta->property(i);
        if(property.isUser(this)) {
            const char *name = property.name();
            QVariant value = property.read(this);

            if(value.canConvert<Template>()) {
                Template temp = value.value<Template>();
                if(temp.path.isEmpty()) {
                    success = false;
                    req << QString(name).replace('_', ' ');
                }
                object[name] = QJsonValue(temp.path);
            } else {
                QString str = value.toString();
                if(str.isEmpty()) {
                    success = false;
                    req << QString(name).replace('_', ' ');
                }
                object[name] = str;
            }
        }
    }
    if(!success) {
        aCritical() << "The required settings was not specified:" << qPrintable(req.join(", "))
                    << "Please specify them in the Project Settings.";
    }

    object[gProjectSdk] = QJsonValue(m_projectSdk);
    object[gProject] = QJsonValue(m_projectId);
    object[gPlatforms] = QJsonArray::fromStringList(m_platforms);
    object[gModules] = QJsonArray::fromStringList(m_modules.toList());

    doc.setObject(object);

    QFile file(m_projectPath.absoluteFilePath());
    if(file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    } else {
        aCritical() << "Unable to save the Project Settings.";
    }
}

QStringList ProjectManager::modules() const {
    return (m_autoModules + m_modules).toList();
}

QStringList ProjectManager::platforms() const {
    QStringList list = m_supportedPlatforms.keys();
    return (m_platforms.isEmpty()) ? list : m_platforms;
}

void ProjectManager::setCurrentPlatform(const QString &platform) {
    m_currentPlatform = (platform.isEmpty()) ?  "desktop" : platform;

    m_importPath = QFileInfo(m_cachePath.absoluteFilePath() +
                             ((platform == nullptr) ? "" : QDir::separator() + m_currentPlatform) +
                             QDir::separator() + gImport);

    QDir dir;
    dir.mkpath(m_importPath.absoluteFilePath());
}

QString ProjectManager::currentPlatformName() const {
    return m_currentPlatform;
}

CodeBuilder *ProjectManager::currentBuilder(const QString &platform) const {
    return m_supportedPlatforms.value(platform.isEmpty() ? m_currentPlatform : platform);
}

void ProjectManager::reportModules(QSet<QString> &modules) {
    m_autoModules += modules;

    QFile file(m_generatedPath.absolutePath() + gModulesFile);
    if(file.open(QIODevice::WriteOnly)) {
        QStringList list = m_autoModules.toList();
        file.write(qPrintable(list.join('\n')));
        file.close();
    }
}

QString ProjectManager::artifact() const {
    return m_artifact;
}

void ProjectManager::setArtifact(const QString &value) {
    m_artifact = value;
}

void ProjectManager::setProjectSdk(const QString &sdk) {
    m_projectSdk = sdk;
}

QVariantList ProjectManager::getModules() {
    QVariantList result;
    for(auto &it : qAsConst(m_modules)) {
        result.push_back(it);
    }
    return result;
}
void ProjectManager::setModules(QVariantList modules) {
    m_modules.clear();
    for(auto &it : modules) {
        m_modules.insert(it.toString());
    }
    emit updated();
}
void ProjectManager::resetModules() {
    if(m_modules.isEmpty()) {
        m_modules.insert(QString());
    }
    emit updated();
}

QVariantList ProjectManager::getPlatforms() {
    QVariantList result;
    for(auto &it : m_platforms) {
        result.push_back(it);
    }
    return result;
}
void ProjectManager::setPlatforms(QVariantList platforms) {
    m_platforms.clear();
    for(auto &it : platforms) {
        m_platforms.push_back(it.toString());
    }
    emit updated();
}
void ProjectManager::resetPlatforms() {
    if(m_platforms.isEmpty()) {
        m_platforms.push_back(QString());
    }
    emit updated();
}
