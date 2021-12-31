#include "qbsbuilder.h"

#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QMetaProperty>

#include <QDebug>

#include <log.h>
#include <config.h>

#include <editor/projectmanager.h>
#include <editor/settingsmanager.h>
#include <editor/pluginmanager.h>

namespace {
    const char *gSdkPath("${sdkPath}");

    const char *gIncludePaths("${includePaths}");
    const char *gLibraryPaths("${libraryPaths}");
    const char *gLibraries("${libraries}");

    const char *gEditorSuffix("-Editor");

    // Android specific
    const char *gManifestFile("${manifestFile}");
    const char *gResourceDir("${resourceDir}");
    const char *gAssetsPaths("${assetsPath}");

    const char *gWinProfile = "Platforms/Windows/QbsProfile";
    const char *gOsxProfile = "Platforms/OSX/QbsProfile";
    const char *gLinProfile = "Platforms/Linux/QbsProfile";

    const char *gAndroidJava("Platforms/Android/Java_Path");
    const char *gAndroidSdk("Platforms/Android/SDK_Path");
    const char *gAndroidNdk("Platforms/Android/NDK_Path");

    const char *gLabel("[QbsBuilder]");

    #ifndef _DEBUG
        const char *gMode = "release";
    #else
        const char *gMode = "debug";
    #endif
};

// generate --generator visualstudio2013

QbsBuilder::QbsBuilder() :
        m_pProcess(new QProcess(this)),
        m_Progress(false) {

    setEnvironment(QStringList(), QStringList(), QStringList());

    m_Settings << "--settings-dir" << QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/..";

    SettingsManager *settings = SettingsManager::instance();
    settings->setProperty(qPrintable(gAndroidJava), QVariant::fromValue(QFileInfo("/")));
    settings->setProperty(qPrintable(gAndroidSdk), QVariant::fromValue(QFileInfo("/")));
    settings->setProperty(qPrintable(gAndroidNdk), QVariant::fromValue(QFileInfo("/")));

#if defined(Q_OS_WIN)
    settings->setProperty(qPrintable(gWinProfile), "");
#elif defined(Q_OS_MAC)
    settings->setProperty(qPrintable(gOsxProfile), "");
#elif defined(Q_OS_UNIX)
    settings->setProperty(qPrintable(gLinProfile), "");
#endif

    connect( m_pProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()) );
    connect( m_pProcess, SIGNAL(readyReadStandardError()), this, SLOT(readError()) );

    connect( m_pProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBuildFinished(int)) );
}

bool QbsBuilder::buildProject() {
    if(m_Outdated && !m_Progress) {
        aInfo() << gLabel << "Build started.";

        ProjectManager *mgr = ProjectManager::instance();
        m_QBSPath = QFileInfo(mgr->sdkPath() + "/tools/qbs/bin/qbs");

        m_Project = mgr->generatedPath() + "/";
        m_pProcess->setWorkingDirectory(m_Project);

        builderInit();
        generateProject();

        QString platform = mgr->currentPlatformName();
        QString product = mgr->projectName();
        QString path = mgr->cachePath() + "/" + platform + "/" + gMode + "/install-root/";
        if(mgr->targetPath().isEmpty()) {
            product += gEditorSuffix;
            m_Artifact = path + mgr->projectName() + gEditorSuffix + gShared;
        } else {
            if(platform == "android") {
                m_Artifact = path + "com." + mgr->projectCompany() + "." + mgr->projectName() + ".apk";
            } else {
                m_Artifact = path + mgr->projectName() + gApplication;
            }
        }
        mgr->setArtifact(m_Artifact);
        QString profile = getProfile(platform);
        QString architecture = getArchitectures(platform).at(0);
        {
            QProcess qbs(this);
            qbs.setWorkingDirectory(m_Project);

            QStringList args;
            args << "resolve" << m_Settings;
            args << "profile:" + profile << QString("config:") + gMode;
            args << "qbs.architecture:" + architecture;

            qbs.start(m_QBSPath.absoluteFilePath(), args);
            if(qbs.waitForStarted() && qbs.waitForFinished()) {
                aInfo() << gLabel << "Resolved:" << qbs.readAllStandardOutput().constData();
            }
        }
        {
            QStringList args;
            args << "build" << m_Settings;
            args << "--build-directory" << "../" + platform;
            args << "--products" << product << "profile:" + profile;
            args << QString("config:") + gMode << "qbs.architecture:" + architecture;

            qDebug() << gLabel << args.join(" ");

            m_pProcess->start(m_QBSPath.absoluteFilePath(), args);
            if(!m_pProcess->waitForStarted()) {
                aError() << "Failed:" << qPrintable(m_pProcess->errorString()) << qPrintable(m_QBSPath.absoluteFilePath());
                return false;
            }
            m_Progress = true;
        }
    }
    return true;
}

void QbsBuilder::builderInit() {
    SettingsManager *settings = SettingsManager::instance();
    if(!checkProfiles()) {
        {
            QProcess qbs(this);
            qbs.setWorkingDirectory(m_Project);
            qbs.start(m_QBSPath.absoluteFilePath(), QStringList() << "setup-toolchains" << "--detect" << m_Settings);
            if(qbs.waitForStarted()) {
                qbs.waitForFinished();
            }
        }
        {
            QString sdk = settings->property(qPrintable(gAndroidSdk)).value<QFileInfo>().filePath();
            if(!sdk.isEmpty()) {
                QStringList args;
                args << "setup-android" << m_Settings;
                args << "--sdk-dir" << sdk;
                args << "--ndk-dir" << settings->property(qPrintable(gAndroidNdk)).value<QFileInfo>().filePath();
                args << "android";

                QProcess qbs(this);
                QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
                env.insert("JAVA_HOME", settings->property(qPrintable(gAndroidJava)).value<QFileInfo>().path());
                qbs.setProcessEnvironment(env);
                qbs.setWorkingDirectory(m_Project);
                qbs.start(m_QBSPath.absoluteFilePath(), args);
                if(qbs.waitForStarted()) {
                    qbs.waitForFinished();
                    qDebug() << gLabel << qbs.readAllStandardError().toStdString().c_str();
                }
            }
        }
    }
}

bool QbsBuilder::checkProfiles() {
    QStringList profiles;
    ProjectManager *mgr = ProjectManager::instance();
    for(QString &p : mgr->platforms()) {
        profiles << getProfile(p);
    }

    QStringList args;
    args << "config" << "--list" << m_Settings;
    QProcess qbs(this);
    qbs.setWorkingDirectory(m_Project);
    qbs.start(m_QBSPath.absoluteFilePath(), args);
    if(qbs.waitForStarted() && qbs.waitForFinished()) {
        QByteArray data = qbs.readAll();
        qDebug() << gLabel << data.toStdString().c_str();
        bool result = true;
        for(QString &it : profiles) {
            result &= data.contains(qPrintable(it));
        }
        return result;
    }
    return false;
}

void QbsBuilder::generateProject() {
    ProjectManager *mgr = ProjectManager::instance();

    aInfo() << gLabel << "Generating project";

    m_Values[gSdkPath] = mgr->sdkPath();
    const QMetaObject *meta = mgr->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property = meta->property(i);
        m_Values[QString("${%1}").arg(property.name())] = property.read(mgr).toString();
    }

    generateLoader(mgr->templatePath(), mgr->modules());

    m_Values[gIncludePaths] = formatList(m_IncludePath);
    m_Values[gLibraryPaths] = formatList(m_LibPath);
    m_Values[gLibraries]    = formatList(m_Libs);
    // Android specific settings
    QFileInfo info(ProjectManager::instance()->manifestFile());
    m_Values[gManifestFile] = info.absoluteFilePath();
    m_Values[gResourceDir]  = info.absolutePath() + "/res";
    m_Values[gAssetsPaths]  = ProjectManager::instance()->importPath();

    updateTemplate(mgr->templatePath() + "/project.qbs", m_Project + mgr->projectName() + ".qbs", m_Values);
}

QString QbsBuilder::getProfile(const QString &platform) const {
    SettingsManager *settings = SettingsManager::instance();
    QString profile;
    if(platform == "desktop") {
    #if defined(Q_OS_WIN)
        profile = settings->property(qPrintable(gWinProfile)).toString();
        if(profile.isEmpty()) {
            profile = "MSVC2015-amd64";
            settings->setProperty(qPrintable(gWinProfile), profile);
        }
    #elif defined(Q_OS_MAC)
        profile = settings->property(qPrintable(gOsxProfile)).toString();
        if(profile.isEmpty()) {
            profile = "xcode-macosx-x86_64";
            settings->setProperty(qPrintable(gOsxProfile), profile);
        }
    #elif defined(Q_OS_UNIX)
        profile = settings->property(qPrintable(gLinProfile)).toString();
        if(profile.isEmpty()) {
            profile = "clang";
            settings->setProperty(qPrintable(gLinProfile), profile);
        }
    #endif

    } else if(platform == "android") {
        profile = "android";
    }
#if defined(Q_OS_MAC)
    if(platform == "ios") {
        profile = "xcode-iphoneos-arm64";
    } else if(platform == "tvos") {
        profile = "xcode-appletvos-arm64";
    }
#endif
    return profile;
}

QStringList QbsBuilder::getArchitectures(const QString &platform) const {
    QStringList architectures;

    if(platform == "desktop") {
#if defined(Q_OS_WIN)
        architectures << "x86_64";
#elif defined(Q_OS_MAC)
        architectures << "x86_64";
#elif defined(Q_OS_UNIX)
        architectures << "x86_64";
#endif
    } else if(platform == "android") {
        architectures << "x86" << "armv7a";
    }
#if defined(Q_OS_MAC)
    if(platform == "ios") {
        architectures << "arm64";
    } else if(platform == "tvos") {
        architectures << "arm64";
    }
#endif
    return architectures;
}

QString QbsBuilder::builderVersion() {
    QProcess qbs(this);
    qbs.setWorkingDirectory(m_Project);
    qbs.start(m_QBSPath.absoluteFilePath(), QStringList() << "--version" );
    if(qbs.waitForStarted() && qbs.waitForFinished()) {
        return qbs.readAll().simplified();
    }
    return QString();
}

void QbsBuilder::onBuildFinished(int exitCode) {
    ProjectManager *mgr = ProjectManager::instance();
    if(exitCode == 0 && mgr->targetPath().isEmpty()) {
        PluginManager::instance()->reloadPlugin(m_Artifact);

        emit buildSuccessful();
    }
    m_Outdated = false;
    m_Progress = false;
}

void QbsBuilder::readOutput() {
    QProcess *p = dynamic_cast<QProcess *>( sender() );
    if(p) {
        parseLogs(p->readAllStandardOutput());
    }
}

void QbsBuilder::readError() {
    QProcess *p = dynamic_cast<QProcess *>( sender() );
    if(p) {
        parseLogs(p->readAllStandardError());
    }
}

void QbsBuilder::parseLogs(const QString &log) {
    QStringList list = log.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

    foreach(QString it, list) {
        if(it.contains(" error ")) {
            aError() << gLabel << qPrintable(it);
        } else if(it.contains(" warning ")) {
            aWarning() << gLabel << qPrintable(it);
        } else {
            aInfo() << gLabel << qPrintable(it);
        }
    }
}

void QbsBuilder::setEnvironment(const QStringList &incp, const QStringList &libp, const QStringList &libs) {
    m_IncludePath = incp;
    m_LibPath = libp;
    m_Libs = libs;
}

bool QbsBuilder::isEmpty() const {
    return !isOutdated();
}

bool QbsBuilder::isPackage(const QString &platform) const {
    if(platform == "desktop") {
        return false;
    }
    return true;
}
