#include "xcodebuilder.h"

#include "projectmanager.h"
#include "platforms/tvos.h"

#include <QProcess>
#include <QMetaProperty>

const QString gSdkPath("${sdkPath}");

const QString gPlatformName("${platformName}");
const QString gDeviceFamily("${deviceFamily}");
const QString gSdkName("${sdkName}");
const QString gAppIcon("${appIcon}");
const QString gLaunchImage("${launchImage}");

// instruments -s devices
// xcodebuild clean archive -project Match3.xcodeproj -scheme Match3 -archivePath $PWD/build/Match3.xcarchive
// xcodebuild -exportArchive -archivePath $PWD/build/Match3.xcarchive -exportOptionsPlist exportOptions.plist -exportPath $PWD/build

XcodeBuilder::XcodeBuilder() :
        m_pProcess(new QProcess(this)),
        m_pMgr(ProjectManager::instance()),
        m_Progress(false) {

    m_Values[gSdkPath] = m_pMgr->sdkPath();
}

QString XcodeBuilder::builderVersion() {
    return QString();
}

bool XcodeBuilder::buildProject() {
    if(m_Outdated && !m_Progress) {
        m_Project = m_pMgr->generatedPath() + "/";
        m_pProcess->setWorkingDirectory(m_Project);

        const QMetaObject *meta = m_pMgr->metaObject();
        for(int i = 0; i < meta->propertyCount(); i++) {
            QMetaProperty property  = meta->property(i);
            m_Values[QString("${%1}").arg(property.name())] = property.read(m_pMgr).toString();
        }

        IPlatform *platform = m_pMgr->currentPlatform();
        if(dynamic_cast<TvOSPlatform *>(platform)) {
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

        generateLoader(m_pMgr->templatePath(), m_pMgr->modules());

        updateTemplate(m_pMgr->templatePath() + "/project.pbxproj", m_Project + m_pMgr->projectName() + ".xcodeproj/project.pbxproj", m_Values);
        updateTemplate(m_pMgr->templatePath() + "/LaunchScreen.storyboard", m_Project + "LaunchScreen.storyboard", m_Values);
        updateTemplate(m_pMgr->templatePath() + "/Info.plist", m_Project + "Info.plist", m_Values);

        m_Outdated = false;
    }
    return true;
}
