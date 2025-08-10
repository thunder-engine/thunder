#include "qbsbuilder.h"

#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QMetaProperty>
#include <QRegularExpression>

#include <log.h>
#include <url.h>
#include <config.h>

#include <editor/projectsettings.h>
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

    const char *gQBSProfile("QBS_Builder/Profile");
    const char *gQBSPath("QBS_Builder/QBS_Path");

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
        m_proxy(new QbsProxy),
        m_process(new QProcess(m_proxy)),
        m_progress(false) {

    setEnvironment(StringList(), StringList(), StringList());

    m_settings.push_back("--settings-dir");
    m_settings.push_back((QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/..").toStdString());

    EditorSettings *settings = EditorSettings::instance();

    settings->registerValue(gAndroidJava, "/", "editor=Path");
    settings->registerValue(gAndroidSdk, "/", "editor=Path");
    settings->registerValue(gAndroidNdk, "/", "editor=Path");

    settings->registerValue(gQBSPath, "/", "editor=Path");

#if defined(Q_OS_WIN)
    settings->registerValue(gQBSProfile, "MSVC2019-x64");
#elif defined(Q_OS_MAC)
    settings->registerValue(gQBSProfile, "xcode-macosx-x86_64");
#elif defined(Q_OS_UNIX)
    settings->registerValue(gQBSProfile, "clang");
#endif

    m_proxy->setBuilder(this);

    Object::connect(settings, _SIGNAL(updated()), this, _SLOT(onApplySettings()));

    QObject::connect( m_process, &QProcess::readyReadStandardOutput, m_proxy, &QbsProxy::readOutput );
    QObject::connect( m_process, &QProcess::readyReadStandardError, m_proxy, &QbsProxy::readError );

    QObject::connect( m_process, SIGNAL(finished(int,QProcess::ExitStatus)), m_proxy, SLOT(onBuildFinished(int)) );
}

bool QbsBuilder::isNative() const {
    return true;
}

bool QbsBuilder::buildProject() {
    if(m_outdated && !m_progress) {
        aInfo() << gLabel << "Build started.";

        m_qbsPath = QFileInfo(EditorSettings::instance()->value(gQBSPath).toString().data());

        ProjectSettings *mgr = ProjectSettings::instance();
        if(m_qbsPath.absoluteFilePath().isEmpty()) {
            TString suffix;
    #if defined(Q_OS_WIN)
            suffix += TString(".") + gApplication;
    #endif
            m_qbsPath = QFileInfo((mgr->sdkPath() + "/tools/qbs/bin/qbs" + suffix).data());
        }

        if(!m_qbsPath.exists()) {
            aCritical() << "Can't find the QBS Tool by the path:" << qPrintable(m_qbsPath.absoluteFilePath());
        }

        m_project = mgr->generatedPath() + "/";
        m_process->setWorkingDirectory(m_project.data());

        builderInit();
        generateProject();

        TString platform = mgr->currentPlatformName();
        TString product = mgr->projectName();
        TString path = mgr->cachePath() + "/" + platform + "/" + gMode + "/install-root/";
        if(mgr->targetPath().isEmpty()) {
            product += gEditorSuffix;
            m_artifact = path + gPrefix + product + "." + gShared;
        } else {
            if(platform == "android") {
                m_artifact = path + "com." + mgr->projectCompany() + "." + mgr->projectName() + ".apk";
            } else {
                m_artifact = path + mgr->projectName() + "." + gApplication;
            }
        }
        mgr->setArtifact(m_artifact.data());
        TString profile = getProfile(platform);
        TString architecture = getArchitectures(platform).front();
        {
            QProcess qbs;
            qbs.setWorkingDirectory(m_project.data());

            QStringList args;
            args << "resolve";
            for(auto it : m_settings) {
                args << it.data();
            }
            args << QString("profile:") + profile.data() << QString("config:") + gMode;
            args << QString("qbs.architecture:") + architecture.data();

            qbs.start(m_qbsPath.absoluteFilePath(), args);
            if(qbs.waitForStarted() && qbs.waitForFinished()) {
                aInfo() << gLabel << "Resolved:" << qbs.readAllStandardOutput().constData();
            }
        }
        {
            QStringList args;
            args << "build";
            for(auto it : m_settings) {
                args << it.data();
            }
            args << "--build-directory" << QString("../") + platform.data();
            args << "--products" << product.data() << QString("profile:") + profile.data();
            args << QString("config:") + gMode << QString("qbs.architecture:") + architecture.data();

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
            QProcess qbs;
            qbs.setWorkingDirectory(m_project.data());
            QStringList args;
            args << "setup-toolchains" << "--detect";
            for(auto it : m_settings) {
                args << it.data();
            }
            qbs.start(m_qbsPath.absoluteFilePath(), args);
            if(qbs.waitForStarted()) {
                qbs.waitForFinished();
            }
        }
        {
            TString sdk = settings->value(gAndroidSdk).toString();
            if(!sdk.isEmpty()) {
                QStringList args;
                args << "setup-android";
                for(auto it : m_settings) {
                    args << it.data();
                }
                args << "--sdk-dir" << sdk.data();
                args << "--ndk-dir" << settings->value(gAndroidNdk).toString().data();
                args << "android";

                QProcess qbs;
                QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
                env.insert("JAVA_HOME", settings->value(gAndroidJava).toString().data());
                qbs.setProcessEnvironment(env);
                qbs.setWorkingDirectory(m_project.data());
                qbs.start(m_qbsPath.absoluteFilePath(), args);
                if(qbs.waitForStarted()) {
                    qbs.waitForFinished();
                    aDebug() << gLabel << qbs.readAllStandardError().toStdString();
                }
            }
        }
    }
}

bool QbsBuilder::checkProfiles() {
    StringList profiles;
    ProjectSettings *mgr = ProjectSettings::instance();
    for(const TString &p : mgr->platforms()) {
        profiles.push_back(getProfile(p));
    }

    QStringList args;
    args << "config" << "--list";
    for(auto it : m_settings) {
        args << it.data();
    }
    QProcess qbs;
    qbs.setWorkingDirectory(m_project.data());
    qbs.start(m_qbsPath.absoluteFilePath(), args);
    if(qbs.waitForStarted() && qbs.waitForFinished()) {
        QByteArray data = qbs.readAll();
        aDebug() << gLabel << data.toStdString().c_str();
        bool result = true;
        for(auto &it : profiles) {
            result &= data.contains(it.data());
        }
        return result;
    }
    return false;
}

void QbsBuilder::generateProject() {
    ProjectSettings *mgr = ProjectSettings::instance();

    aInfo() << gLabel << "Generating project";

    m_values[gSdkPath] = mgr->sdkPath();
    const MetaObject *meta = mgr->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);
        m_values[QString("${%1}").arg(property.name()).toStdString()] = property.read(mgr).toString();
    }

    generateLoader(mgr->templatePath(), mgr->modules());

    m_values[gIncludePaths] = formatList(m_includePath);
    m_values[gLibraryPaths] = formatList(m_libPath);
    m_values[gLibraries]    = formatList(m_libs);
    // Android specific settings
    QFileInfo info(mgr->manifestFile().data());
    m_values[gManifestFile] = info.absoluteFilePath().toStdString();
    m_values[gResourceDir]  = (info.absolutePath() + "/res").toStdString();
    m_values[gAssetsPaths]  = mgr->importPath();

    updateTemplate(":/templates/project.qbs", m_project + mgr->projectName() + ".qbs");

#if defined(Q_OS_WIN)
    TString architecture = getArchitectures(mgr->currentPlatformName()).front();

    QProcess qbs;
    qbs.setWorkingDirectory(m_project.data());
    qbs.start(m_qbsPath.absoluteFilePath(), QStringList() << "generate" << "-g" << "visualstudio2015"
                                                          << QString("config:") + gMode << QString("qbs.architecture:") + architecture.data());
    if(qbs.waitForStarted()) {
        qbs.waitForFinished();
    }
#endif
}

TString QbsBuilder::getProfile(const TString &platform) const {
    TString profile;
    if(platform == "desktop") {
        profile = EditorSettings::instance()->value(gQBSProfile).toString();
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

StringList QbsBuilder::getArchitectures(const TString &platform) const {
    StringList architectures;

    if(platform == "desktop") {
#if defined(Q_OS_WIN)
        architectures.push_back("x86_64");
#elif defined(Q_OS_MAC)
        architectures.push_back("x86_64");
#elif defined(Q_OS_UNIX)
        architectures.push_back("x86_64");
#endif
    } else if(platform == "android") {
        architectures.push_back("x86");
        architectures.push_back("armv7a");
    }
#if defined(Q_OS_MAC)
    if(platform == "ios") {
        architectures.push_back("arm64");
    } else if(platform == "tvos") {
        architectures.push_back("arm64");
    }
#endif
    return architectures;
}

void QbsBuilder::onBuildFinished(int exitCode) {
    ProjectSettings *mgr = ProjectSettings::instance();
    if(exitCode == 0 && mgr->targetPath().isEmpty()) {
        PluginManager::instance()->reloadPlugin(m_artifact.data());

        buildSuccessful();
    }
    m_outdated = false;
    m_progress = false;
}

void QbsBuilder::onApplySettings() {
    m_qbsPath = EditorSettings::instance()->value(gQBSPath).value<QFileInfo>();
}

void QbsBuilder::parseLogs(const QString &log) {
    QStringList list = log.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);

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

void QbsBuilder::setEnvironment(const StringList &incp, const StringList &libp, const StringList &libs) {
    m_includePath = incp;
    m_libPath = libp;
    m_libs = libs;
}

bool QbsBuilder::isBundle(const TString &platform) const {
    if(platform == "desktop") {
        return false;
    }
    return true;
}
