#include "emscriptenbuilder.h"

#include <QDir>

#include <os/processenvironment.h>

#include <log.h>
#include <url.h>
#include <file.h>
#include <config.h>

#include <editor/projectsettings.h>
#include <editor/editorsettings.h>
#include <editor/pluginmanager.h>

namespace {
    const char *gSdkPath("${sdkPath}");

    const char *gEmscriptenPath("Emscripten_Builder/Emscripten_SDK");

    const char *gLabel("[EmscriptenBuilder]");

};

EmscriptenBuilder::EmscriptenBuilder() {

    EditorSettings *settings = EditorSettings::instance();

    settings->registerValue(gEmscriptenPath, "/", "editor=Path");

    connect(settings, _SIGNAL(updated()), this, _SLOT(onApplySettings()));

    connect(&m_process, _SIGNAL(readyReadStandardOutput()), this, _SLOT(readOutput()) );
    connect(&m_process, _SIGNAL(readyReadStandardError()), this, _SLOT(readError()) );
    connect(&m_process, _SIGNAL(finished(int)), this, _SLOT(onBuildFinished(int)) );

    TString sdk(ProjectSettings::instance()->sdkPath());

    m_includePath.push_back(sdk + "/include/engine");
    m_includePath.push_back(sdk + "/include/modules");
    m_includePath.push_back(sdk + "/include/next");
    m_includePath.push_back(sdk + "/include/next/math");
    m_includePath.push_back(sdk + "/include/next/core");

    m_libPath.push_back(sdk + "/emscripten/x86/static");

    m_libs.push_back("engine");
    m_libs.push_back("next");
    m_libs.push_back("physfs");
    m_libs.push_back("zlib");
    m_libs.push_back("glfm");
    m_libs.push_back("bullet");
    m_libs.push_back("bullet3");
    m_libs.push_back("rendergl");
    m_libs.push_back("freetype");
    m_libs.push_back("uikit");
    m_libs.push_back("media");
    m_libs.push_back("vorbis");
    m_libs.push_back("vorbisfile");
    m_libs.push_back("ogg");
    m_libs.push_back("angel");
    m_libs.push_back("angelscript");
}

bool EmscriptenBuilder::isEmpty() const {
    return (ProjectSettings::instance()->currentPlatformName() != "webgl");
}

bool EmscriptenBuilder::buildProject() {
    if(m_outdated && !m_process.isRunning()) {
        aInfo() << gLabel << "Build started.";

        onApplySettings();

        ProjectSettings *mgr = ProjectSettings::instance();

        m_project = mgr->generatedPath() + "/";

        m_process.setWorkingDirectory(m_project);

        generateProject();

        m_artifact = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/release";

        QDir dir;
        dir.mkpath(m_artifact.data());

        mgr->setArtifact(m_artifact);
        {
            StringList args;

            for(auto &it : m_includePath) {
                args.push_back(TString("-I") + it);
            }

            for(auto &it : m_libPath) {
                args.push_back(TString("-L") + it);
            }

            for(auto &it : m_libs) {
                args.push_back(TString("-l") + it);
            }

            args.push_back("-std=c++20");
            args.push_back("-sMIN_WEBGL_VERSION=2");
            args.push_back("-sALLOW_MEMORY_GROWTH");
            args.push_back("-DTHUNDER_MOBILE");

            args.push_back("-O3");

            args.push_back("application.cpp");
            args.push_back("-o");
            args.push_back((m_artifact + "/application.js").data());

            args.push_back("--preload-file");
            args.push_back("../webgl/assets@/");

            if(m_process.start(m_binary, args)) {
                if(!m_process.waitForStarted()) {
                    aError() << "Failed to start process";
                    return false;
                }
            }
        }
    }
    return true;
}

void EmscriptenBuilder::generateProject() {
    ProjectSettings *mgr = ProjectSettings::instance();

    aInfo() << gLabel << "Generating project";

    m_values[gSdkPath] = mgr->sdkPath();
    const MetaObject *meta = mgr->metaObject();
    for(int i = 0; i < meta->propertyCount(); i++) {
        MetaProperty property = meta->property(i);
        m_values[QString("${%1}").arg(property.name()).toStdString()] = property.read(mgr).toString();
    }

    generateLoader(mgr->templatePath(), mgr->modules());
}

void EmscriptenBuilder::onBuildFinished(int exitCode) {
    ProjectSettings *mgr = ProjectSettings::instance();
    if(exitCode == 0) {
        TString targetFile(mgr->artifact() + "/application.html");

        File::remove(targetFile);
        QFile::copy(":/application.html", targetFile.data());
        QFile::setPermissions(targetFile.data(), QFileDevice::WriteOwner);

        aInfo() << gLabel << "Build finished";

        if(mgr->targetPath().isEmpty()) {
            buildSuccessful();
        }
    }
    m_outdated = false;
}

void EmscriptenBuilder::readOutput() {
    parseLogs(m_process.readAllStandardOutput());
}

void EmscriptenBuilder::readError() {
    parseLogs(m_process.readAllStandardError());
}

void EmscriptenBuilder::onApplySettings() {
    TString sdk(EditorSettings::instance()->value(gEmscriptenPath).toString());

    if(!File::exists(sdk)) {
        aCritical() << "Can't find the Emscripten SDK by the path:" << sdk;
    }

    ProcessEnvironment env = ProcessEnvironment::systemEnvironment();

#ifdef _WIN32
    sdk.replace('/', '\\');

    env.insert("EMSDK", sdk);
    env.insert("EMSDK_PYTHON", sdk + "\\python\\3.9.2-nuget_64bit\\python.exe");
    env.insert("EMSDK_NODE", sdk + "\\node\\16.20.0_64bit\\bin\\node.exe");
    env.insert("JAVA_HOME", sdk + "\\java\\8.152_64bit");

    m_binary = sdk.toStdString() + "\\upstream\\emscripten\\emcc.bat";

    TString path = env.value("PATH") + ";";
    path += sdk + ";";
    path += sdk + "\\upstream\\emscripten;";
    path += sdk + "\\node\\16.20.0_64bit\\bin";

    env.insert("PATH", path.data());
#endif

    m_process.setProcessEnvironment(env);
}

void EmscriptenBuilder::parseLogs(const TString &log) {
    for(const TString &it : log.split("\r\n")) {
        if(it.contains(" error ") || it.contains(" error:")) {
            aError() << gLabel << it;
        } else if(it.contains(" warning ") || it.contains(" warning:")) {
            aWarning() << gLabel << it;
        } else {
            aInfo() << gLabel << it;
        }
    }
}

bool EmscriptenBuilder::isBundle(const TString &platform) const {
    if(platform == platforms().front()) {
        return true;
    }
    return false;
}
