#include "vsbuilder.h"

#include <log.h>
#include <url.h>
#include <config.h>
#include <file.h>

#include <editor/projectsettings.h>
#include <editor/editorsettings.h>
#include <os/processenvironment.h>

namespace {
    const char *gEditorSuffix("-editor");
    const char *gMsBuildPath("Builder/VisualStudio/MSBuildPath");

#ifndef _DEBUG
    const char *gMode("Release");
#else
    const char *gMode("Debug");
#endif
}

VsBuilder::VsBuilder() {
    setName("[VsBuilder]");

    connect(&m_process, _SIGNAL(finished(int)), this, _SLOT(onBuildFinished(int)));

    EditorSettings *settings = Editor::settings();
    settings->registerValue(gMsBuildPath, "", "editor=Path");
    m_vsPath = settings->value(gMsBuildPath).toString();

    m_filePref = "    <ClCompile Include=\""; m_fileSuff = "\" />"; m_fileSep = "";
    m_incPathSep = ";";
    m_libPathSep = ";";
    m_libsPref = ""; m_libsSuff = ".lib"; m_libsSep = ";";
    m_defSep = ";";

    ProjectSettings *mgr = Editor::project();
    TString sdk = mgr->sdkPath();
    if(mgr->targetPath().isEmpty()) {
        m_libPath = {
            sdk + "/windows/x86_64/lib",
            sdk + "/windows/x86_64/bin",
            sdk + "/windows/x86_64/bin/plugins"
        };
    } else {
        m_libPath = { sdk + "/windows/x86_64/static" };
    }

    m_defines = {
        TString("COMPANY_NAME=\"%1\"").arg(mgr->projectCompany()),
        TString("PRODUCT_NAME=\"%1\"").arg(mgr->projectName()),
        TString("PRODUCT_VERSION=\"%1\"").arg(mgr->projectVersion())
    };
}

bool VsBuilder::buildProject() {
    if(m_outdated && !m_process.isRunning()) {
        m_vsPath = Editor::settings()->value(gMsBuildPath).toString();
        if(m_vsPath.isEmpty() || !File::exists(m_vsPath)) {
            TString vswherePath("C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe");
            if(File::exists(vswherePath)) {
                Process vswhere;
                TString vswhereProgram = TString("\"") + vswherePath + "\"";
                if(vswhere.start(vswhereProgram, {"-latest", "-property", "installationPath"}) &&
                   vswhere.waitForStarted() && vswhere.waitForFinished()) {
                    TString installationPath = vswhere.readAllStandardOutput().trimmed();
                    if(!installationPath.isEmpty()) {
                        installationPath.replace('\\', '/');
                        m_vsPath = installationPath + "/Msbuild/Current/Bin/MSBuild.exe";
                        Editor::settings()->setValue(gMsBuildPath, m_vsPath);
                    }
                }
            }
        }

        if(m_vsPath.isEmpty() || !File::exists(m_vsPath)) {
            aError() << name() << "Unable to find MSBuild.exe at:" << m_vsPath;
            return false;
        }

        generateProject();

        ProjectSettings *mgr = Editor::project();

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

    ProjectSettings *mgr = Editor::project();

    m_project = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/";

    updateTemplate(":/templates/windows/project.sln", m_project + mgr->projectName() + ".sln");
    updateTemplate(":/templates/windows/project.vcxproj", m_project + "project.vcxproj", true);
    updateTemplate(":/templates/windows/project-editor.vcxproj", m_project + "project-editor.vcxproj", true);
}
