#include "xcodebuilder.h"

#include <editor/projectsettings.h>

#include <QProcess>
#include <QMetaProperty>
#include <QRegularExpression>

#include <log.h>

#include <editor/pluginmanager.h>

namespace {
    const char *gPlatformName("{platformName}");
    const char *gDeviceFamily("{deviceFamily}");
    const char *gSdkName("{sdkName}");
    const char *gAppIcon("{appIcon}");
    const char *gLaunchImage("{launchImage}");
};

// instruments -s devices
// xcodebuild clean archive -project Match3.xcodeproj -scheme Match3 -archivePath $PWD/build/Match3.xcarchive
// xcodebuild -exportArchive -archivePath $PWD/build/Match3.xcarchive -exportOptionsPlist exportOptions.plist -exportPath $PWD/build

XcodeBuilder::XcodeBuilder() {
    setName("[XcodeBuilder]");

}

bool XcodeBuilder::buildProject() {
    if(m_outdated && !m_process.isRunning()) {
        ProjectSettings *mgr = ProjectSettings::instance();

        m_project = mgr->generatedPath() + "/";
        m_process.setWorkingDirectory(m_project);

        generateProject();

        m_outdated = false;
    }

    return true;
}

void XcodeBuilder::generateProject() {
    NativeCodeBuilder::generateProject();

    ProjectSettings *mgr = ProjectSettings::instance();

    if(mgr->currentPlatformName() == "tvos") {
        m_values[gSdkName] = "appletvos";
        m_values[gPlatformName] = "tvos";
        m_values[gDeviceFamily] = "3";
        m_values[gAppIcon] = "App Icon & Top Shelf Image";
        m_values[gLaunchImage] = "LaunchImage";
    } else {
        m_values[gSdkName] = "iphoneos";
        m_values[gPlatformName] = "ios";
        m_values[gDeviceFamily] = "1,2";
        m_values[gAppIcon] = "AppIcon";
        m_values[gLaunchImage] = "";
    }

    updateTemplate(":/templates/project.pbxproj", m_project + mgr->projectName() + ".xcodeproj/project.pbxproj");
    updateTemplate(":/templates/LaunchScreen.storyboard", m_project + "LaunchScreen.storyboard");
    updateTemplate(":/templates/Info.plist", m_project + "Info.plist");
}
