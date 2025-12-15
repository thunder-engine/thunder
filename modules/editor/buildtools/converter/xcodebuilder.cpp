#include "xcodebuilder.h"

#include <editor/projectsettings.h>

#include <log.h>
#include <file.h>

namespace {
    const char *gPlatformName("${platformName}");
    const char *gDeviceFamily("${deviceFamily}");
    const char *gSdkName("${sdkName}");
    const char *gAppIcon("${appIcon}");
    const char *gLaunchImage("${launchImage}");

    const char *gPlatformsPath("${platformsPath}");
    const char *gCachePath("${cachePath}");
#ifdef NDEBUG
    const char *gMode("Release");
#else
    const char *gMode("Debug");
#endif
};

XcodeBuilder::XcodeBuilder() {
    setName("[XcodeBuilder]");

    connect(&m_process, _SIGNAL(finished(int)), this, _SLOT(onBuildFinished(int)));
}

bool XcodeBuilder::buildProject() {
    if(m_outdated && !m_process.isRunning()) {
        m_process.setWorkingDirectory(m_project);

        ProjectSettings *mgr = ProjectSettings::instance();

        m_values[gPlatformsPath] = mgr->platformsPath();
        m_values[gCachePath] = mgr->cachePath();

        generateProject();

        TString path = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/";

        StringList args;
        args.push_back("clean");
        if(mgr->targetPath().isEmpty()) {
            args.push_back("build");
        } else {
            args.push_back("archive");
        }
        args.push_back("-project");
        args.push_back(mgr->projectName() + ".xcodeproj");
        args.push_back("-scheme");
        if(mgr->targetPath().isEmpty()) {
            args.push_back(mgr->projectName() + "-editor");
            args.push_back("-configuration");
            args.push_back(gMode);

            m_artifact = path + "build/lib" + mgr->projectName() + "-editor.dylib";
        } else {
            args.push_back(mgr->projectName());
            args.push_back("-archivePath");

            m_artifact = path + "build/" + mgr->projectName() + ".xcarchive";
            args.push_back(m_artifact);
        }

        mgr->setArtifacts({ m_artifact });

        m_process.setWorkingDirectory(m_project);

        if(m_process.start("xcodebuild", args) && m_process.waitForStarted()) {
            aInfo() << name() << "Build Project.";
            return m_process.waitForFinished();
        }
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
        m_values[gAppIcon] = "Brand Assets";
        m_values[gLaunchImage] = "";
    } else if(mgr->currentPlatformName() == "ios") {
        m_values[gSdkName] = "iphoneos";
        m_values[gPlatformName] = "ios";
        m_values[gDeviceFamily] = "1,2";
        m_values[gAppIcon] = "AppIcon";
        m_values[gLaunchImage] = "";
    } else {
        m_values[gSdkName] = "macosx";
        m_values[gDeviceFamily] = "";
        m_values[gPlatformName] = "macos";
        m_values[gAppIcon] = "AppIcon";
        m_values[gLaunchImage] = "";
    }

    m_project = mgr->cachePath() + "/" + mgr->currentPlatformName() + "/";

    if(mgr->currentPlatformName() == "macos") {
        updateTemplate(":/templates/xcode/desktop.pbxproj", m_project + mgr->projectName() + ".xcodeproj/project.pbxproj");
        updateTemplate(":/templates/xcode/macos.plist", m_project + "Info.plist");
    } else {
        updateTemplate(":/templates/xcode/mobile.pbxproj", m_project + mgr->projectName() + ".xcodeproj/project.pbxproj");
        updateTemplate(":/templates/xcode/LaunchScreen.storyboard", m_project + "LaunchScreen.storyboard");
        updateTemplate(":/templates/xcode/ios.plist", m_project + "Info.plist");
    }

    StringList res;

    if(mgr->currentPlatformName() == "tvos") {
        res = {
            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon - App Store.imagestack/Back.imagestacklayer/Contents.json",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon - App Store.imagestack/Back.imagestacklayer/Content.imageset/Contents.json",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon - App Store.imagestack/Front.imagestacklayer/Contents.json",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon - App Store.imagestack/Front.imagestacklayer/Content.imageset/Contents.json",

            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon.imagestack/Contents.json",

            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon.imagestack/Back.imagestacklayer/Contents.json",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon.imagestack/Back.imagestacklayer/Content.imageset/Contents.json",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon.imagestack/Back.imagestacklayer/Content.imageset/1.png",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon.imagestack/Back.imagestacklayer/Content.imageset/2.png",

            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon.imagestack/Front.imagestacklayer/Contents.json",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon.imagestack/Front.imagestacklayer/Content.imageset/Contents.json",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon.imagestack/Front.imagestacklayer/Content.imageset/1.png",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/App Icon.imagestack/Front.imagestacklayer/Content.imageset/2.png",

            "/tvos/Assets.xcassets/Brand Assets.brandassets/Top Shelf Image Wide.imageset/Contents.json",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/Top Shelf Image Wide.imageset/1.png",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/Top Shelf Image Wide.imageset/2.png",

            "/tvos/Assets.xcassets/Brand Assets.brandassets/Top Shelf Image.imageset/Contents.json",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/Top Shelf Image.imageset/1.png",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/Top Shelf Image.imageset/2.png",
            "/tvos/Assets.xcassets/Brand Assets.brandassets/Contents.json",

            "/tvos/Assets.xcassets/Contents.json",
        };
    } if(mgr->currentPlatformName() == "ios") {
        res = {
            "/ios/Assets.xcassets/Contents.json",
            "/ios/Assets.xcassets/AppIcon.appiconset/Contents.json",
            "/ios/Assets.xcassets/AppIcon.appiconset/Dark.png",
            "/ios/Assets.xcassets/AppIcon.appiconset/Default.png",
            "/ios/Assets.xcassets/AppIcon.appiconset/Tinted.png"
        };
    } if(mgr->currentPlatformName() == "macos") {
        res = {
            "/macos/Assets.xcassets/Contents.json",
            "/macos/Assets.xcassets/AppIcon.appiconset/Contents.json",
            "/macos/Assets.xcassets/AppIcon.appiconset/1024.png"
        };
    }

    for(auto &it : res) {
        copyTempalte(TString(":/templates/xcode") + it, mgr->platformsPath() + it);
    }
}
