#include "qbsbuilder.h"

#include <QStandardPaths>

#include <log.h>
#include <url.h>
#include <config.h>
#include <file.h>

#include <os/processenvironment.h>

#include <editor/projectsettings.h>
#include <editor/editorsettings.h>

namespace {
    const char *gEditorSuffix("-editor");

    // Android specific
    const char *gManifestFile("${manifestFile}");
    const char *gResourceDir("${resourceDir}");
    const char *gAssetsPaths("${assetsPath}");

    const char *gQBSProfile("Builder/QBS/Profile");
    const char *gQBSPath("Builder/QBS/Path");

    const char *gAndroidJava("Builder/Android/Java_Path");
    const char *gAndroidSdk("Builder/Android/SDK_Path");
    const char *gAndroidNdk("Builder/Android/NDK_Path");

    #ifndef _DEBUG
        const char *gMode = "release";
    #else
        const char *gMode = "debug";
    #endif
};

QbsBuilder::QbsBuilder() {
    setName("[QbsBuilder]");

    m_settings.push_back("--settings-dir");
    m_settings.push_back((QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/..").toStdString());

    EditorSettings *settings = EditorSettings::instance();

    settings->registerValue(gAndroidJava, "", "editor=Path");
    settings->registerValue(gAndroidSdk, "", "editor=Path");
    settings->registerValue(gAndroidNdk, "", "editor=Path");

    settings->registerValue(gQBSPath, "", "editor=FilePath");

#if defined(Q_OS_WIN)
    settings->registerValue(gQBSProfile, "MSVC2019-x64");
#elif defined(Q_OS_MAC)
    settings->registerValue(gQBSProfile, "xcode-macosx-x86_64");
#elif defined(Q_OS_UNIX)
    settings->registerValue(gQBSProfile, "clang");
#endif
}

bool QbsBuilder::buildProject() {
    if(m_outdated && !m_process.isRunning()) {
        aInfo() << name() << "Build started.";

        m_qbsPath = EditorSettings::instance()->value(gQBSPath).toString();

        ProjectSettings *mgr = ProjectSettings::instance();
        if(m_qbsPath.isEmpty()) {
            TString suffix;
    #if defined(Q_OS_WIN)
            suffix += TString(".") + gApplication;
    #endif
            m_qbsPath = mgr->sdkPath() + "/tools/qbs/bin/qbs" + suffix;
        }

        if(!File::exists(m_qbsPath)) {
            aCritical() << name() << "Can't find the QBS Tool by the path:" << m_qbsPath;
        }

        m_project = mgr->generatedPath() + "/";
        m_process.setWorkingDirectory(m_project);

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
        mgr->setArtifacts({ m_artifact });
        TString profile = getProfile(platform);
        TString architecture = getArchitectures(platform).front();
        {
            StringList args;
            args.push_back("resolve");
            for(auto &it : m_settings) {
                args.push_back(it);
            }
            args.push_back(TString("profile:") + profile);
            args.push_back(TString("config:") + gMode);
            args.push_back(TString("qbs.architecture:") + architecture);

            Process qbs;
            qbs.setWorkingDirectory(m_project);

            if(qbs.start(m_qbsPath, args) && qbs.waitForStarted() && qbs.waitForFinished()) {
                aInfo() << name() << "Resolved:" << qbs.readAllStandardOutput();
            }
        }
        {
            StringList args;
            args.push_back("build");
            for(auto &it : m_settings) {
                args.push_back(it);
            }
            args.push_back("--build-directory");
            args.push_back(TString("../") + platform);

            args.push_back("--products");
            args.push_back(product);
            args.push_back(TString("profile:") + profile);

            args.push_back(TString("config:") + gMode);
            args.push_back(TString("qbs.architecture:") + architecture);

            if(m_process.start(m_qbsPath, args) && !m_process.waitForStarted()) {
                aError() << name() << "Failed:" << m_process.readAllStandardError() << m_qbsPath;
                return false;
            }
        }
    }
    return true;
}

void QbsBuilder::builderInit() {
    EditorSettings *settings = EditorSettings::instance();
    if(!checkProfiles()) {
        {
            StringList args;
            args.push_back("setup-toolchains");
            args.push_back("--detect");
            for(auto &it : m_settings) {
                args.push_back(it);
            }

            Process qbs;
            qbs.setWorkingDirectory(m_project);
            if(qbs.start(m_qbsPath, args) && qbs.waitForStarted()) {
                qbs.waitForFinished();
            }
        }
        {
            TString sdk = settings->value(gAndroidSdk).toString();
            if(!sdk.isEmpty()) {
                StringList args;
                args.push_back("setup-android");
                for(auto &it : m_settings) {
                    args.push_back(it);
                }
                args.push_back("--sdk-dir");
                args.push_back(sdk);

                args.push_back("--ndk-dir");
                args.push_back(settings->value(gAndroidNdk).toString());
                args.push_back("android");

                Process qbs;
                ProcessEnvironment env = ProcessEnvironment::systemEnvironment();
                env.insert("JAVA_HOME", settings->value(gAndroidJava).toString());
                qbs.setProcessEnvironment(env);

                qbs.setWorkingDirectory(m_project);
                if(qbs.start(m_qbsPath, args) && qbs.waitForStarted()) {
                    qbs.waitForFinished();
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

    StringList args;
    args.push_back("config");
    args.push_back("--list");
    for(auto &it : m_settings) {
        args.push_back(it);
    }

    Process qbs;
    qbs.setWorkingDirectory(m_project);

    if(qbs.start(m_qbsPath, args) && qbs.waitForStarted() && qbs.waitForFinished()) {
        TString data = qbs.readAllStandardOutput();
        parseLogs(data);

        bool result = true;
        for(auto &it : profiles) {
            result &= data.contains(it);
        }
        return result;
    }
    return false;
}

void QbsBuilder::generateProject() {
    NativeCodeBuilder::generateProject();

    // Android specific settings
    ProjectSettings *mgr = ProjectSettings::instance();
    m_values[gManifestFile] = mgr->manifestFile();
    m_values[gResourceDir]  = Url(mgr->manifestFile()).dir() + "/res";
    m_values[gAssetsPaths]  = mgr->importPath();

    updateTemplate(":/templates/project.qbs", m_project + mgr->projectName() + ".qbs");

#if defined(Q_OS_WIN)
    StringList args;
    args.push_back("generate");
    args.push_back("-g");
    args.push_back("visualstudio2015");
    args.push_back(TString("config:") + gMode);
    args.push_back(TString("qbs.architecture:") + getArchitectures(mgr->currentPlatformName()).front());

    Process qbs;
    qbs.setWorkingDirectory(m_project);

    if(qbs.start(m_qbsPath, args) && qbs.waitForStarted()) {
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

    if(platform == "ios") {
        profile = "xcode-iphoneos-arm64";
    } else if(platform == "tvos") {
        profile = "xcode-appletvos-arm64";
    }

    return profile;
}

StringList QbsBuilder::getArchitectures(const TString &platform) const {
    StringList architectures;

    if(platform == "desktop") {
        architectures.push_back("x86_64");
    } else if(platform == "android") {
        architectures.push_back("x86");
        architectures.push_back("armv7a");
    }

    if(platform == "ios" || platform == "tvos") {
        architectures.push_back("arm64");
    }

    return architectures;
}
