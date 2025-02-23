#include "xcodebuilder.h"

#include <editor/projectsettings.h>

#include <QProcess>
#include <QMetaProperty>

namespace {
    const char *gSdkPath("${sdkPath}");

    const char *gPlatformName("${platformName}");
    const char *gDeviceFamily("${deviceFamily}");
    const char *gSdkName("${sdkName}");
    const char *gAppIcon("${appIcon}");
    const char *gLaunchImage("${launchImage}");
};

// instruments -s devices
// xcodebuild clean archive -project Match3.xcodeproj -scheme Match3 -archivePath $PWD/build/Match3.xcarchive
// xcodebuild -exportArchive -archivePath $PWD/build/Match3.xcarchive -exportOptionsPlist exportOptions.plist -exportPath $PWD/build

XcodeBuilder::XcodeBuilder() :
        m_process(new QProcess(this)),
        m_progress(false) {

}

QString XcodeBuilder::builderVersion() {
    return QString();
}

bool XcodeBuilder::isNative() const {
    return true;
}

bool XcodeBuilder::buildProject() {
    if(m_outdated && !m_progress) {
        ProjectSettings *mgr = ProjectSettings::instance();

        m_values[gSdkPath] = mgr->sdkPath();

        m_project = mgr->generatedPath() + "/";
        m_process->setWorkingDirectory(m_project);

        const QMetaObject *meta = mgr->metaObject();
        for(int i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty property = meta->property(i);
            m_values[QString("${%1}").arg(property.name())] = property.read(mgr).toString();
        }

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

        generateLoader(mgr->templatePath(), mgr->modules());

        updateTemplate(":/templates/project.pbxproj", m_project + mgr->projectName() + ".xcodeproj/project.pbxproj", m_values);
        updateTemplate(":/templates/LaunchScreen.storyboard", m_project + "LaunchScreen.storyboard", m_values);
        updateTemplate(":/templates/Info.plist", m_project + "Info.plist", m_values);

        m_outdated = false;
    }
    return true;
}
