#include "projectmanager.h"

#include <QUuid>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QMetaProperty>
#include <QCoreApplication>

#include "config.h"

#include "platforms/desktop.h"
#include "platforms/android.h"

const QString gCompany("Company");
const QString gProject("ProjectId");

ProjectManager *ProjectManager::m_pInstance   = nullptr;

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

    m_SDKPath       = QFileInfo(dir.absolutePath());
    m_ResourcePath  = QFileInfo(sdkPath() + "/resources");
    m_TemplatePath  = QFileInfo(resourcePath() + "/editor/templates");

    m_MyProjectsPath  = QFileInfo(dir.absolutePath());

    setSupportedPlatform(new DesktopPlatform);
    setSupportedPlatform(new AndroidPlatform);
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
    m_ProjectPath   = QFileInfo(project);

    if(!target.isEmpty()) {
        QDir dir;
        dir.mkpath(target);
    }
    m_TargetPath    = QFileInfo(target);

    loadSettings();

    m_ProjectName   = m_ProjectPath.completeBaseName();

    m_ContentPath   = QFileInfo(m_ProjectPath.absolutePath() + QDir::separator() + gContent);
    m_PluginsPath   = QFileInfo(m_ProjectPath.absolutePath() + QDir::separator() + gPlugins);
    m_CachePath     = QFileInfo(m_ProjectPath.absolutePath() + QDir::separator() + gCache);

    m_IconPath      = QFileInfo(m_CachePath.absoluteFilePath() + QDir::separator() + gIcons);
    m_GeneratedPath = QFileInfo(m_CachePath.absoluteFilePath() + QDir::separator() + gGenerated);

    m_ManifestFile  = QFileInfo(m_ProjectPath.absolutePath() + QDir::separator() + gPlatforms + "/android/AndroidManifest.xml");

    QDir dir;
    dir.mkpath(m_ContentPath.absoluteFilePath());
    dir.mkpath(m_IconPath.absoluteFilePath());
    dir.mkpath(m_GeneratedPath.absoluteFilePath());
    dir.mkpath(m_PluginsPath.absoluteFilePath());

    setCurrentPlatform();
}

void ProjectManager::loadSettings() {
    QFile file(m_ProjectPath.absoluteFilePath());
    if(file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if(!doc.isNull()) {
            const QMetaObject *meta = metaObject();
            QJsonObject object  = doc.object();
            foreach(const QString &it, object.keys()) {
                int index   = meta->indexOfProperty(qPrintable(it));
                if(index > -1) {
                    QMetaProperty property  = meta->property(index);
                    QVariant value  = object.value(it).toVariant();

                    if(property.userType() == qMetaTypeId<Template>()) {
                        value   = QVariant::fromValue<Template>(Template(value.toString(), IConverter::ContentMap));
                    }
                    property.write(this, value);
                }
            }
            {
                QJsonObject::iterator it = object.find(gProject);
                if(it != doc.object().end()) {
                    m_ProjectId = it.value().toString();
                } else {
                    m_ProjectId = QUuid::createUuid().toString();
                    saveSettings();
                }
            }
            {
                QJsonObject::iterator it = object.find(gPlatforms);
                if(it != doc.object().end()) {
                    foreach(auto platform, it.value().toArray()) {
                        m_Platforms << platform.toString();
                    }
                }
            }
        }
    }
}

void ProjectManager::saveSettings() {
    QJsonDocument doc;

    const QMetaObject *meta = metaObject();

    QJsonObject object;
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property  = meta->property(i);
        if(property.isUser(this)) {
            const char *name = property.name();
            QVariant value = property.read(this);
            if(value.canConvert<Template>()) {
                object[name] = QJsonValue(value.value<Template>().path);
            } else {
                object[name] = QJsonValue(value.toString());
            }
        }
    }
    object[gProject] = QJsonValue(m_ProjectId);
    if(!m_Platforms.isEmpty()) {
        object[gPlatforms] = QJsonArray::fromStringList(m_Platforms);
    }

    doc.setObject(object);

    QFile file(m_ProjectPath.absoluteFilePath());
    if(file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

QStringList ProjectManager::platforms() const {
    QStringList list = m_SupportedPlatforms.keys();
    return (m_Platforms.isEmpty()) ? list : m_Platforms;
}

IPlatform *ProjectManager::supportedPlatform(const QString &platform) {
    return m_SupportedPlatforms[platform];
}

void ProjectManager::setSupportedPlatform(IPlatform *platform) {
    m_SupportedPlatforms[platform->name()] = platform;
}

IPlatform *ProjectManager::currentPlatform() const {
    return m_pCurrentPlatform;
}

void ProjectManager::setCurrentPlatform(const QString &platform) {
    m_pCurrentPlatform = (platform.isEmpty()) ?  m_SupportedPlatforms["desktop"] : m_SupportedPlatforms[platform];

    m_ImportPath = QFileInfo(m_CachePath.absoluteFilePath() +
                             ((platform == nullptr) ? "" : QDir::separator() + m_pCurrentPlatform->name()) +
                             QDir::separator() + gImport);

    QDir dir;
    dir.mkpath(m_ImportPath.absoluteFilePath());
}
