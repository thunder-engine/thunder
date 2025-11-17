#include "vsbuilder.h"

#include <log.h>
#include <url.h>
#include <config.h>
#include <file.h>

#include <editor/projectsettings.h>
#include <os/processenvironment.h>

namespace {
    const char *gEditorSuffix("-editor");

#ifndef _DEBUG
    const char *gMode("Release");
#else
    const char *gMode("Debug");
#endif
}

VsBuilder::VsBuilder() {
    setName("[VsBuilder]");

    connect(&m_process, _SIGNAL(finished(int)), this, _SLOT(onBuildFinished(int)));

    TString vswherePath("C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe");

    if(File::exists(vswherePath)) {
        Process vswhere;
        if(vswhere.start(vswherePath, {"-latest", "-property", "installationPath"})) {
            if(vswhere.waitForStarted() && vswhere.waitForFinished()) {
                m_vsPath = vswhere.readAllStandardOutput().trimmed();
                if(!m_vsPath.isEmpty()) {
                    m_vsPath.replace('\\', '/');
                    m_vsPath += "/Msbuild/Current/Bin/MSBuild.exe";
                }
            }
        }
    }

    m_filePref = "    <ClCompile Include=\""; m_fileSuff = "\" />"; m_fileSep = "";
}

bool VsBuilder::buildProject() {
    if(m_outdated && !m_process.isRunning()) {
        generateProject();

        ProjectSettings *mgr = ProjectSettings::instance();

        StringList args;

        TString product = mgr->projectName();
        TString path = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/" + gMode + "/";
        if(mgr->targetPath().isEmpty()) {
            product += gEditorSuffix;
            m_artifact = path + gPrefix + product + gShared;
            args = {"project-editor.vcxproj", TString("/p:Configuration=") + gMode, "/p:Platform=\"x64\"", "/verbosity:minimal", "/nologo"};
        } else {
            m_artifact = path + product + gApplication;
            args = {"project.vcxproj", TString("/p:Configuration=") + gMode, "/p:Platform=\"x64\"", "/verbosity:minimal", "/nologo"};
        }
        mgr->setArtifacts({ m_artifact });

        ProcessEnvironment env = ProcessEnvironment::systemEnvironment();
        env.insert("VSLANG", "1033");

        m_process.setProcessEnvironment(env);
        m_process.setWorkingDirectory(m_project);
        if(!m_process.start(m_vsPath, args) || !m_process.waitForStarted()) {
            aError() << name() << "Failed to start process";
            return false;
        }
    }
    return true;
}

void VsBuilder::generateProject() {
    NativeCodeBuilder::generateProject();

    ProjectSettings *mgr = ProjectSettings::instance();

    m_project = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/";

    updateTemplate(":/templates/windows/project.sln", m_project + mgr->projectName() + ".sln");
    updateTemplate(":/templates/windows/project.vcxproj", m_project + "project.vcxproj");
    updateTemplate(":/templates/windows/project-editor.vcxproj", m_project + "project-editor.vcxproj");
}
