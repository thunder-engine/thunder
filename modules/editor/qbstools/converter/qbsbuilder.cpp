#include "qbsbuilder.h"

#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QMetaProperty>

#include <log.h>
#include <config.h>

#include <editor/projectmanager.h>
#include <editor/editorsettings.h>
#include <editor/pluginmanager.h>

namespace {
    const char *gSdkPath("${sdkPath}");

    const char *gIncludePaths("${includePaths}");
    const char *gLibraryPaths("${libraryPaths}");
    const char *gLibraries("${libraries}");

    const char *gEditorSuffix("-editor");

    // Android specific
    const char *gManifestFile("${manifestFile}");
    const char *gResourceDir("${resourceDir}");
    const char *gAssetsPaths("${assetsPath}");

    const char *gQBSProfile = "QBS_Builder/Profile";
    const char *gQBSPath = "QBS_Builder/QBS_Path";

    const char *gAndroidJava("QBS_Builder/Android/Java_Path");
    const char *gAndroidSdk("QBS_Builder/Android/SDK_Path");
    const char *gAndroidNdk("QBS_Builder/Android/NDK_Path");

    const char *gLabel("[QbsBuilder]");

    #ifndef _DEBUG
        const char *gMode = "release";
    #else
        const char *gMode = "debug";
    #endif
};

QbsBuilder::QbsBuilder() :
        m_process(new QProcess(this)),
        m_progress(false) {

    setEnvironment(QStringList(), QStringList(), QStringList());

    m_settings << "--settings-dir" << QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/..";

    EditorSettings *settings = EditorSettings::instance();

    m_javaPath = settings->value(gAndroidJava, QVariant::fromValue(QFileInfo("/"))).toString();
    m_androidSDKPath = settings->value(gAndroidSdk, QVariant::fromValue(QFileInfo("/"))).toString();
    m_androidNDKPath = settings->value(gAndroidNdk, QVariant::fromValue(QFileInfo("/"))).toString();

    m_qbsPath = settings->value(gQBSPath, QVariant::fromValue(QFileInfo("/"))).toString();

    connect(settings, &EditorSettings::updated, this, &QbsBuilder::onApplySettings);

    connect( m_process, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()) );
    connect( m_process, SIGNAL(readyReadStandardError()), this, SLOT(readError()) );

    connect( m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onBuildFinished(int)) );
}

bool QbsBuilder::isNative() const {
    return true;
}

bool QbsBuilder::buildProject() {
    if(m_outdated && !m_progress) {
        aInfo() << gLabel << "Build started.";

        ProjectManager *mgr = ProjectManager::instance();
        if(m_qbsPath.absoluteFilePath().isEmpty()) {
            QString suffix;
    #if defined(Q_OS_WIN)
            suffix += ".exe";
    #endif
            m_qbsPath = ProjectManager::instance()->sdkPath() + "/tools/qbs/bin/qbs" + suffix;
        }

        if(!m_qbsPath.exists()) {
            aCritical() << "Can't find the QBS Tool by the path:" << qPrintable(m_qbsPath.absoluteFilePath());
        }

        m_project = mgr->generatedPath() + "/";
        m_process->setWorkingDirectory(m_project);

        builderInit();
        generateProject();

        QString platform = mgr->currentPlatformName();
        QString product = mgr->projectName();
        QString path = mgr->cachePath() + "/" + platform + "/" + gMode + "/install-root/";
        if(mgr->targetPath().isEmpty()) {
            product += gEditorSuffix;
            m_artifact = path + gPrefix + mgr->projectName() + gEditorSuffix + gShared;
        } else {
            if(platform == "android") {
                m_artifact = path + "com." + mgr->projectCompany() + "." + mgr->projectName() + ".apk";
            } else {
                m_artifact = path + mgr->projectName() + gApplication;
            }
        }
        mgr->setArtifact(m_artifact);
        QString profile = getProfile(platform);
        QString architecture = getArchitectures(platform).at(0);
        {
            QProcess qbs(this);
            qbs.setWorkingDirectory(m_project);

            QStringList args;
            args << "resolve" << m_settings;
            args << "profile:" + profile << QString("config:") + gMode;
            args << "qbs.architecture:" + architecture;

            qbs.start(m_qbsPath.absoluteFilePath(), args);
            if(qbs.waitForStarted() && qbs.waitForFinished()) {
                aInfo() << gLabel << "Resolved:" << qbs.readAllStandardOutput().constData();
            }
        }
        {
            QStringList args;
            args << "build" << m_settings;
            args << "--build-directory" << "../" + platform;
            args << "--products" << product << "profile:" + profile;
            args << QString("config:") + gMode << "qbs.architecture:" + architecture;

            aInfo() << gLabel << qPrintable(args.join(" "));

            m_process->start(m_qbsPath.absoluteFilePath(), args);
            if(!m_process->waitForStarted()) {
                aError() << "Failed:" << qPrintable(m_process->errorString()) << qPrintable(m_qbsPath.absoluteFilePath());
                return false;
            }
            m_progress = true;
        }
    }
    return true;
}

void QbsBuilder::builderInit() {
    EditorSettings *settings = EditorSettings::instance();
    if(!checkProfiles()) {
        {
            QProcess qbs(this);
            qbs.setWorkingDirectory(m_project);
            qbs.start(m_qbsPath.absoluteFilePath(), QStringList() << "setup-toolchains" << "--detect" << m_settings);
            if(qbs.waitForStarted()) {
                qbs.waitForFinished();
            }
        }
        {
            QString sdk = settings->value(qPrintable(gAndroidSdk)).value<QFileInfo>().filePath();
            if(!sdk.isEmpty()) {
                QStringList args;
                args << "setup-android" << m_settings;
                args << "--sdk-dir" << sdk;
                args << "--ndk-dir" << settings->value(qPrintable(gAndroidNdk)).value<QFileInfo>().filePath();
                args << "android";

                QProcess qbs(this);
                QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
                env.insert("JAVA_HOME", settings->value(qPrintable(gAndroidJava)).value<QFileInfo>().path());
                qbs.setProcessEnvironment(env);
                qbs.setWorkingDirectory(m_project);
                qbs.start(m_qbsPath.absoluteFilePath(), args);
                if(qbs.waitForStarted()) {
                    qbs.waitForFinished();
                    aDebug() << gLabel << qbs.readAllStandardError().toStdString().c_str();
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
    args << "config" << "--list" << m_settings;
    QProcess qbs(this);
    qbs.setWorkingDirectory(m_project);
    qbs.start(m_qbsPath.absoluteFilePath(), args);
    if(qbs.waitForStarted() && qbs.waitForFinished()) {
        QByteArray data = qbs.readAll();
        aDebug() << gLabel << data.toStdString().c_str();
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

    m_values[gSdkPath] = mgr->sdkPath();
    const QMetaObject *meta = mgr->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        QMetaProperty property = meta->property(i);
        m_values[QString("${%1}").arg(property.name())] = property.read(mgr).toString();
    }

    generateLoader(mgr->templatePath(), mgr->modules());

    m_values[gIncludePaths] = formatList(m_includePath);
    m_values[gLibraryPaths] = formatList(m_libPath);
    m_values[gLibraries]    = formatList(m_libs);
    // Android specific settings
    QFileInfo info(ProjectManager::instance()->manifestFile());
    m_values[gManifestFile] = info.absoluteFilePath();
    m_values[gResourceDir]  = info.absolutePath() + "/res";
    m_values[gAssetsPaths]  = ProjectManager::instance()->importPath();

    updateTemplate(":/templates/project.qbs", m_project + mgr->projectName() + ".qbs", m_values);

#if defined(Q_OS_WIN)
    QString architecture = getArchitectures(mgr->currentPlatformName()).at(0);

    QProcess qbs(this);
    qbs.setWorkingDirectory(m_project);
    qbs.start(m_qbsPath.absoluteFilePath(), QStringList() << "generate" << "-g" << "visualstudio2015"
                                                          << QString("config:") + gMode << "qbs.architecture:" + architecture);
    if(qbs.waitForStarted()) {
        qbs.waitForFinished();
    }
#endif
}

QString QbsBuilder::getProfile(const QString &platform) const {
    QString profile;
    if(platform == "desktop") {
        EditorSettings *settings = EditorSettings::instance();
    #if defined(Q_OS_WIN)
        profile = settings->value(gQBSProfile, "MSVC2015-amd64").toString();
    #elif defined(Q_OS_MAC)
        profile = settings->value(gQBSProfile, "xcode-macosx-x86_64").toString();
    #elif defined(Q_OS_UNIX)
        profile = settings->value(gQBSProfile, "clang").toString();
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
    qbs.setWorkingDirectory(m_project);
    qbs.start(m_qbsPath.absoluteFilePath(), QStringList() << "--version" );
    if(qbs.waitForStarted() && qbs.waitForFinished()) {
        return qbs.readAll().simplified();
    }
    return QString();
}

void QbsBuilder::onBuildFinished(int exitCode) {
    ProjectManager *mgr = ProjectManager::instance();
    if(exitCode == 0 && mgr->targetPath().isEmpty()) {
        PluginManager::instance()->reloadPlugin(m_artifact);

        emit buildSuccessful();
    }
    m_outdated = false;
    m_progress = false;
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

void QbsBuilder::onApplySettings() {
    m_qbsPath = EditorSettings::instance()->value(gQBSPath).value<QFileInfo>();
}

void QbsBuilder::parseLogs(const QString &log) {
    QStringList list = log.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

    foreach(QString it, list) {
        if(it.contains(" error ") || it.contains(" error:", Qt::CaseInsensitive)) {
            aError() << gLabel << qPrintable(it);
        } else if(it.contains(" warning ") || it.contains(" warning:", Qt::CaseInsensitive)) {
            aWarning() << gLabel << qPrintable(it);
        } else {
            aInfo() << gLabel << qPrintable(it);
        }
    }
}

void QbsBuilder::setEnvironment(const QStringList &incp, const QStringList &libp, const QStringList &libs) {
    m_includePath = incp;
    m_libPath = libp;
    m_libs = libs;
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
