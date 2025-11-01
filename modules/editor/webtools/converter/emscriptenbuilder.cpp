#include "emscriptenbuilder.h"

#include <os/processenvironment.h>

#include <QFile>

#include <log.h>
#include <file.h>

#include <editor/projectsettings.h>
#include <editor/editorsettings.h>

namespace {
    const char *gEmscriptenPath("Builder/Emscripten/Path");
};

EmscriptenBuilder::EmscriptenBuilder() {
    setName("[EmscriptenBuilder]");

    EditorSettings *settings = EditorSettings::instance();

    settings->registerValue(gEmscriptenPath, "", "editor=Path");

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
        aInfo() << name() << "Build started.";

        m_emPath = EditorSettings::instance()->value(gEmscriptenPath).toString();

        ProcessEnvironment env = ProcessEnvironment::systemEnvironment();

#ifdef _WIN32
        env.insert("EMSDK", m_emPath);
        env.insert("EMSDK_PYTHON", m_emPath + "\\python\\3.9.2-nuget_64bit\\python.exe");
        env.insert("EMSDK_NODE", m_emPath + "\\node\\16.20.0_64bit\\bin\\node.exe");
        env.insert("JAVA_HOME", m_emPath + "\\java\\8.152_64bit");

        m_binary = m_emPath + "/upstream/emscripten/emcc.bat";

        TString path = env.value("PATH") + ";";
        path += m_emPath + ";";
        path += m_emPath + "\\upstream\\emscripten;";
        path += m_emPath + "\\node\\16.20.0_64bit\\bin";

        env.insert("PATH", path);
#endif

        m_process.setProcessEnvironment(env);

        if(m_emPath.isEmpty() || !File::exists(m_binary)) {
            aError() << name() << "Unable to find Emscripten SDK at:" << m_emPath;
            return false;
        }

        ProjectSettings *mgr = ProjectSettings::instance();

        m_project = mgr->generatedPath() + "/";

        generateProject();

        m_artifact = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/release";

        File::mkPath(m_artifact);

        mgr->setArtifacts({m_artifact + "/application.data",
                           m_artifact + "/application.html",
                           m_artifact + "/application.js",
                           m_artifact + "/application.wasm"});

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
        args.push_back(m_artifact + "/application.js");

        args.push_back("--preload-file");
        args.push_back("../webgl/assets@/");

        m_process.setWorkingDirectory(m_project);
        if(m_process.start(m_binary, args) && !m_process.waitForStarted()) {
            aError() << name() << "Failed to start process";
            return false;
        }
    }
    return true;
}

void EmscriptenBuilder::onBuildFinished(int exitCode) {
    if(exitCode == 0) {
        TString targetFile(m_artifact + "/application.html");

        File::remove(targetFile);
        QFile::copy(":/application.html", targetFile.data());
        QFile::setPermissions(targetFile.data(), QFileDevice::WriteOwner);
    }
    NativeCodeBuilder::onBuildFinished(exitCode);
}
