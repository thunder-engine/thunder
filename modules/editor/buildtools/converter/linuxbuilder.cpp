#include "linuxbuilder.h"

#include <os/processenvironment.h>

#include <log.h>
#include <config.h>
#include <file.h>

#include <editor/projectsettings.h>
#include <editor/editorsettings.h>

namespace {
    const char *gEditorSuffix("-editor");

#ifndef NDEBUG
    const char *gMode("release");
#else
    const char *gMode("debug");
#endif
}

LinuxBuilder::LinuxBuilder() {
    setName("[LinuxBuilder]");

    connect(&m_process, _SIGNAL(finished(int)), this, _SLOT(onBuildFinished(int)));

    m_libs.push_back("vorbis");
    m_libs.push_back("vorbisfile");
    m_libs.push_back("ogg");
    m_libs.push_back("zlib");
}

bool LinuxBuilder::isEmpty() const {
    return (ProjectSettings::instance()->currentPlatformName() != "linux");
}

bool LinuxBuilder::buildProject() {
    if(m_outdated && !m_process.isRunning()) {
        aInfo() << name() << "Build started.";

        generateProject();

        ProjectSettings *mgr = ProjectSettings::instance();

        TString product = mgr->projectName();
        TString path = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/" + gMode + "/";

        File::mkPath(path);

        m_libPath.clear();
        if(mgr->targetPath().isEmpty()) {
            m_artifact = path + gPrefix + product + gShared;
        } else {
            m_libPath.push_back(ProjectSettings::instance()->sdkPath() + "/linux/x86/static");
            m_artifact = path + product;
        }

        mgr->setArtifacts({ m_artifact });

        StringList args;
        for(auto &it : m_incPath) {
            args.push_back(TString("-I") + it);
        }

        for(auto &it : m_libPath) {
            args.push_back(TString("-L") + it);
        }

        for(auto &it : m_libs) {
            args.push_back(TString("-l") + it);
        }

        args.push_back("-std=c++20");
#ifdef NDEBUG
        args.push_back("-O3");
#endif
        if(mgr->targetPath().isEmpty()) {
            args.push_back("plugin.cpp");
        } else {
            args.push_back("application.cpp");
        }
        args.push_back("-o");
        args.push_back(m_artifact);

        m_process.setWorkingDirectory(m_project);
        if(m_process.start("clang", args) && !m_process.waitForStarted()) {
            aError() << name() << "Failed to start process.";
            return false;
        }
    }
    return true;
}
