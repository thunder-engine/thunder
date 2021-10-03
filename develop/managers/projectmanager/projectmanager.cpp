#include "projectmanager.h"

#include <QUuid>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QDebug>

#include <QMetaProperty>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QSettings>

#include <log.h>

#include "config.h"

#include "platforms/desktop.h"
#include "platforms/android.h"
#include "platforms/ios.h"
#include "platforms/tvos.h"

const QString gCompany("Company");
const QString gProject("ProjectId");
const QString gProjects("Projects");

ProjectManager *ProjectManager::m_pInstance = nullptr;

ProjectManager::ProjectManager() :
        m_Builder(new QProcess(this)) {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#if defined(Q_OS_MAC)
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#endif

    m_SDKPath = QFileInfo(dir.absolutePath());
    m_ResourcePath = QFileInfo(sdkPath() + "/resources");
    m_TemplatePath = QFileInfo(resourcePath() + "/editor/templates");

    QSettings settings(COMPANY_NAME, EDITOR_NAME);
    QString path = settings.value(gProjects, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    m_MyProjectsPath = QFileInfo(path);

    setSupportedPlatform(new DesktopPlatform);
    setSupportedPlatform(new AndroidPlatform);
#if defined(Q_OS_MAC)
    setSupportedPlatform(new IOSPlatform);
    setSupportedPlatform(new TvOSPlatform);
#endif

    connect(m_Builder, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBuildFinished(int,QProcess::ExitStatus)));

    connect(m_Builder, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()));
    connect(m_Builder, SIGNAL(readyReadStandardError()), this, SLOT(readError()));
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
    m_ProjectPath = QFileInfo(project);

    if(!target.isEmpty()) {
        QDir dir;
        dir.mkpath(target);
    }
    m_TargetPath = QFileInfo(target);

    loadSettings();

    m_ProjectName = m_ProjectPath.completeBaseName();

    m_ContentPath = QFileInfo(m_ProjectPath.absolutePath() + QDir::separator() + gContent);
    m_PluginsPath = QFileInfo(m_ProjectPath.absolutePath() + QDir::separator() + gPlugins);
    m_CachePath = QFileInfo(m_ProjectPath.absolutePath() + QDir::separator() + gCache);

    m_IconPath = QFileInfo(m_CachePath.absoluteFilePath() + QDir::separator() + gThumbnails);
    m_GeneratedPath = QFileInfo(m_CachePath.absoluteFilePath() + QDir::separator() + gGenerated);

    m_ManifestFile = QFileInfo(m_ProjectPath.absolutePath() + QDir::separator() + gPlatforms + "/android/AndroidManifest.xml");

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

        m_ProjectId = QUuid::createUuid().toString();
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
                QJsonObject::iterator it = object.find(gProject);
                if(it != doc.object().end()) {
                    m_ProjectId = it.value().toString();
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
            {
                QJsonObject::iterator it = object.find(gModules);
                if(it != doc.object().end()) {
                    foreach(auto module, it.value().toArray()) {
                        m_Modules << module.toString();
                    }
                }
            }
        }

        if(m_Modules.indexOf("RenderGL") == -1) {
            m_Modules.append("RenderGL");
        }

        saveSettings();
    }
}

void ProjectManager::saveSettings() {
    QJsonDocument doc;

    const QMetaObject *meta = metaObject();

    QJsonObject object;
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property = meta->property(i);
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

    if(!m_Modules.isEmpty()) {
        object[gModules] = QJsonArray::fromStringList(m_Modules);
    }

    doc.setObject(object);

    QFile file(m_ProjectPath.absoluteFilePath());
    if(file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void ProjectManager::build(QString platform) {
    QString dir = QFileDialog::getExistingDirectory(nullptr, tr("Select Target Directory"),
                                                    "",
                                                    QFileDialog::ShowDirsOnly |
                                                    QFileDialog::DontResolveSymlinks);

    if(!dir.isEmpty()) {
        ProjectManager *mgr = ProjectManager::instance();

        QStringList args;
        args << "-s" << mgr->projectPath() << "-t" << dir;

        if(!platform.isEmpty()) {
            args << "-p" << platform;
        }

        qDebug() << args.join(" ");

        m_Builder->start("Builder", args);
        if(!m_Builder->waitForStarted()) {
            Log(Log::ERR) << qPrintable(m_Builder->errorString());
        }
    }
}

void ProjectManager::onBuildFinished(int exitCode, QProcess::ExitStatus) {
    if(exitCode == 0) {
        Log(Log::INF) << "Build Finished";
    } else {
        Log(Log::ERR) << "Build Failed";
    }
}

void ProjectManager::readOutput() {
    emit readBuildLogs(m_Builder->readAllStandardOutput());
}

void ProjectManager::readError() {
    emit readBuildLogs(m_Builder->readAllStandardError());
}

QStringList ProjectManager::modules() const {
    return m_Modules;
}

QStringList ProjectManager::platforms() const {
    QStringList list = m_SupportedPlatforms.keys();
    return (m_Platforms.isEmpty()) ? list : m_Platforms;
}

Platform *ProjectManager::supportedPlatform(const QString &platform) {
    return m_SupportedPlatforms[platform];
}

void ProjectManager::setSupportedPlatform(Platform *platform) {
    m_SupportedPlatforms[platform->name()] = platform;
}

Platform *ProjectManager::currentPlatform() const {
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
