#include "qbsbuilder.h"

#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QMetaProperty>

#include <QDynamicPropertyChangeEvent>

#include <QDebug>

#include <log.h>
#include <config.h>

#include <projectmanager.h>
#include <settingsmanager.h>
#include <assetmanager.h>
#include <pluginmanager.h>
#include <platforms/android.h>

const QString gSdkPath("${sdkPath}");

const QString gIncludePaths("${includePaths}");
const QString gLibraryPaths("${libraryPaths}");
const QString gLibraries("${libraries}");

const QString gEditorSuffix("-Editor");

const QString gProfile("profile");
const QString gArchitectures("architectures");

// Android specific
const QString gManifestFile("${manifestFile}");
const QString gResourceDir("${resourceDir}");
const QString gAssetsPaths("${assetsPath}");

const QString gAndroidJava("Platforms/Android/Java_Path");
const QString gAndroidSdk("Platforms/Android/SDK_Path");
const QString gAndroidNdk("Platforms/Android/NDK_Path");

#ifndef _DEBUG
    const QString gMode = "release";
#else
    const QString gMode = "debug";
#endif

// generate --generator visualstudio2013

QbsBuilder::QbsBuilder() :
        m_pProcess(new QProcess(this)),
        m_pMgr(ProjectManager::instance()),
        m_Progress(false) {

    m_Values[gSdkPath] = m_pMgr->sdkPath();
    m_QBSPath   = QFileInfo(m_pMgr->sdkPath() + "/tools/qbs/bin/qbs");

    setEnvironment(QStringList(), QStringList(), QStringList());

    m_Settings << "--settings-dir" << QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/..";

    SettingsManager *settings = SettingsManager::instance();
    settings->setProperty(qPrintable(gAndroidJava), QVariant::fromValue(FilePath("/")));
    settings->setProperty(qPrintable(gAndroidSdk), QVariant::fromValue(FilePath("/")));
    settings->setProperty(qPrintable(gAndroidNdk), QVariant::fromValue(FilePath("/")));

    connect( m_pProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()) );
    connect( m_pProcess, SIGNAL(readyReadStandardError()), this, SLOT(readError()) );

    connect( m_pProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBuildFinished(int)) );

    IPlatform *platform = m_pMgr->supportedPlatform("desktop");
    if(platform) {
        QString profile;
    #if defined(Q_OS_WIN)
        profile = "MSVC2015-amd64";
    #elif defined(Q_OS_MAC)
        profile = "xcode-macosx-x86_64";
    #elif defined(Q_OS_UNIX)
        profile = "clang";
    #endif
        platform->setProperty(qPrintable(gProfile), profile);
        QStringList architectures;
#if defined(Q_OS_WIN)
        architectures << "x86_64";
#elif defined(Q_OS_MAC)
        architectures << "x86_64";
#elif defined(Q_OS_UNIX)
        architectures << "x86_64";
#endif
        platform->setProperty(qPrintable(gArchitectures), architectures);
    }

    platform = m_pMgr->supportedPlatform("android");
    if(platform) {
        platform->setProperty(qPrintable(gProfile), "android");
        platform->setProperty(qPrintable(gArchitectures), QStringList() << "x86" << "armv7a");
    }
#if defined(Q_OS_MAC)
    platform = m_pMgr->supportedPlatform("ios");
    if(platform) {
        platform->setProperty(qPrintable(gProfile), "xcode-iphoneos-arm64");
        platform->setProperty(qPrintable(gArchitectures), QStringList() << "arm64");
    }
    platform = m_pMgr->supportedPlatform("tvos");
    if(platform) {
        platform->setProperty(qPrintable(gProfile), "xcode-appletvos-arm64");
        platform->setProperty(qPrintable(gArchitectures), QStringList() << "arm64");
    }
#endif
}

void QbsBuilder::generateProject() {
    const QMetaObject *meta = m_pMgr->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property  = meta->property(i);
        m_Values[QString("${%1}").arg(property.name())] = property.read(m_pMgr).toString();
    }

    generateLoader(m_pMgr->templatePath(), m_pMgr->modules());

    m_Values[gIncludePaths]   = formatList(m_IncludePath);
    m_Values[gLibraryPaths]   = formatList(m_LibPath);
    m_Values[gLibraries]      = formatList(m_Libs);
    // Android specific settings
    QFileInfo info(ProjectManager::instance()->manifestFile());
    m_Values[gManifestFile]   = info.absoluteFilePath();
    m_Values[gResourceDir]    = info.absolutePath() + "/res";
    m_Values[gAssetsPaths]    = ProjectManager::instance()->importPath();

    updateTemplate(m_pMgr->templatePath() + "/project.qbs", m_Project + m_pMgr->projectName() + ".qbs", m_Values);
}

bool QbsBuilder::buildProject() {
    if(m_Outdated && !m_Progress) {
        m_Project = m_pMgr->generatedPath() + "/";
        m_pProcess->setWorkingDirectory(m_Project);

        builderInit();
        generateProject();

        IPlatform *platform = m_pMgr->currentPlatform();
        QString product = m_pMgr->projectName();
        QString path = m_pMgr->cachePath() + "/" + platform->name() + "/" + gMode + "/install-root/";
        if(m_pMgr->targetPath().isEmpty()) {
            product += gEditorSuffix;
            m_Artifact = path + m_pMgr->projectName() + gEditorSuffix + gShared;
        } else {
            if(dynamic_cast<AndroidPlatform *>(platform) != nullptr) {
                m_Artifact = path + "com." + m_pMgr->projectCompany() + "." + m_pMgr->projectName() + ".apk";
            } else {
                m_Artifact = path + m_pMgr->projectName() + gApplication;
            }
        }
        AssetManager::instance()->setArtifact(m_Artifact);
        QString profile = platform->property(qPrintable(gProfile)).toString();
        QString architecture = platform->property(qPrintable(gArchitectures)).toStringList().at(0);
        {
            QProcess qbs(this);
            qbs.setWorkingDirectory(m_Project);

            QStringList args;
            args << "resolve" << m_Settings;
            args << "profile:" + profile << "config:" + gMode;
            args << "qbs.architecture:" + architecture;

            qbs.start(m_QBSPath.absoluteFilePath(), args);
            if(qbs.waitForStarted() && qbs.waitForFinished()) {
                Log(Log::INF) << "Resolved:" << qbs.readAllStandardOutput().constData();
            }
        }
        {
            QStringList args;
            args << "build" << m_Settings;
            if(platform) {
                args << "--build-directory" << "../" + platform->name();
            }
            args << "--products" << product << "profile:" + profile;
            args << "config:" + gMode << "qbs.architecture:" + architecture;

            qDebug() << args.join(" ");

            m_pProcess->start(m_QBSPath.absoluteFilePath(), args);
            if(!m_pProcess->waitForStarted()) {
                Log(Log::ERR) << "Failed:" << qPrintable(m_pProcess->errorString()) << qPrintable(m_QBSPath.absoluteFilePath());
                return false;
            }
            m_Progress = true;
        }
    }
    return true;
}

void QbsBuilder::onBuildFinished(int exitCode) {
    if(exitCode == 0 && m_pMgr->targetPath().isEmpty()) {
        PluginManager::instance()->reloadPlugin(m_Artifact);
    }
    m_Outdated = false;
    m_Progress = false;
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

void QbsBuilder::builderInit() {
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
            SettingsManager *settings = SettingsManager::instance();

            QString sdk = settings->property(qPrintable(gAndroidSdk)).value<FilePath>().filePath();
            if(!sdk.isEmpty()) {
                QStringList args;
                args << "setup-android" << m_Settings;
                args << "--sdk-dir" << sdk;
                args << "--ndk-dir" << settings->property(qPrintable(gAndroidNdk)).value<FilePath>().filePath();
                args << "android";

                QProcess qbs(this);
                QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
                env.insert("JAVA_HOME", settings->property(qPrintable(gAndroidJava)).value<FilePath>().path());
                qbs.setProcessEnvironment(env);
                qbs.setWorkingDirectory(m_Project);
                qbs.start(m_QBSPath.absoluteFilePath(), args);
                if(qbs.waitForStarted()) {
                    qbs.waitForFinished();
                    qDebug() << qbs.readAllStandardError().toStdString().c_str();
                }
            }
        }
    }
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
            Log(Log::ERR) << qPrintable(it);
        } else if(it.contains(" warning ")) {
            Log(Log::WRN) << qPrintable(it);
        } else {
            Log(Log::INF) << qPrintable(it);
        }
    }
}

bool QbsBuilder::checkProfiles() {
    QStringList profiles;
    for(QString p : m_pMgr->platforms()) {
        IPlatform *platform = m_pMgr->supportedPlatform(p);
        if(platform) {
            profiles << platform->property(qPrintable(gProfile)).toString();
        }
    }

    QStringList args;
    args << "config" << "--list" << m_Settings;
    QProcess qbs(this);
    qbs.setWorkingDirectory(m_Project);
    qbs.start(m_QBSPath.absoluteFilePath(), args);
    if(qbs.waitForStarted() && qbs.waitForFinished()) {
        QByteArray data = qbs.readAll();
        qDebug() << data.toStdString().c_str();
        bool result = true;
        for(QString it : profiles) {
            result &= data.contains(qPrintable(it));
        }
        return result;
    }
    return false;
}

void QbsBuilder::setEnvironment(const QStringList &incp, const QStringList &libp, const QStringList &libs) {
    m_IncludePath = incp;
    m_LibPath = libp;
    m_Libs = libs;
}

bool QbsBuilder::isEmpty() const {
    return !isOutdated();
}
