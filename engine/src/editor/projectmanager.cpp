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

#include <editor/pluginmanager.h>

namespace {
    const char *gCompany("Company");
    const char *gProject("ProjectId");
    const char *gProjects("Projects");
};

ProjectManager *ProjectManager::m_pInstance = nullptr;

ProjectManager::ProjectManager() :
        m_builder(new QProcess(this)) {
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

    connect(m_builder, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBuildFinished(int,QProcess::ExitStatus)));

    connect(m_builder, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()));
    connect(m_builder, SIGNAL(readyReadStandardError()), this, SLOT(readError()));
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

    object[gProject] = QJsonValue(m_projectId);
    if(!m_platforms.isEmpty()) {
        object[gPlatforms] = QJsonArray::fromStringList(m_platforms);
    }

    if(!m_modules.isEmpty()) {
        object[gModules] = QJsonArray::fromStringList(m_modules.toList());
    }

    doc.setObject(object);

    QFile file(m_projectPath.absoluteFilePath());
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

        m_builder->start("Builder", args);
        if(!m_builder->waitForStarted()) {
            Log(Log::ERR) << qPrintable(m_builder->errorString());
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
    emit readBuildLogs(m_builder->readAllStandardOutput());
}

void ProjectManager::readError() {
    emit readBuildLogs(m_builder->readAllStandardError());
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
}

QString ProjectManager::artifact() const {
    return m_artifact;
}

void ProjectManager::setArtifact(const QString &value) {
    m_artifact = value;
}
