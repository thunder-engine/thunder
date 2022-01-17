#include "xcodebuilder.h"

#include <editor/projectmanager.h>

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
        m_pProcess(new QProcess(this)),
        m_Progress(false) {

}

QString XcodeBuilder::builderVersion() {
    return QString();
}

bool XcodeBuilder::buildProject() {
    if(m_Outdated && !m_Progress) {
        ProjectManager *mgr = ProjectManager::instance();

        m_Values[gSdkPath] = mgr->sdkPath();

        m_Project = mgr->generatedPath() + "/";
        m_pProcess->setWorkingDirectory(m_Project);

        const QMetaObject *meta = mgr->metaObject();
        for(int i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty property = meta->property(i);
            m_Values[QString("${%1}").arg(property.name())] = property.read(mgr).toString();
        }

        if(mgr->currentPlatformName() == "tvos") {
            m_Values[gSdkName] = "appletvos";
            m_Values[gPlatformName] = "tvos";
            m_Values[gDeviceFamily] = "3";
            m_Values[gAppIcon] = "App Icon & Top Shelf Image";
            m_Values[gLaunchImage] = "LaunchImage";
        } else {
            m_Values[gSdkName] = "iphoneos";
            m_Values[gPlatformName] = "ios";
            m_Values[gDeviceFamily] = "1,2";
            m_Values[gAppIcon] = "AppIcon";
            m_Values[gLaunchImage] = "";
        }

        generateLoader(mgr->templatePath(), mgr->modules());

        updateTemplate(":/templates/project.pbxproj", m_Project + mgr->projectName() + ".xcodeproj/project.pbxproj", m_Values);
        updateTemplate(":/templates/LaunchScreen.storyboard", m_Project + "LaunchScreen.storyboard", m_Values);
        updateTemplate(":/templates/Info.plist", m_Project + "Info.plist", m_Values);

        m_Outdated = false;
    }
    return true;
}
