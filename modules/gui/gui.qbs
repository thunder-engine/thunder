import qbs

Project {
    id: gui
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/systems/*.cpp",

        "includes/systems/*.h"
    ]

    property stringList incPaths: [
        "includes",
        "../../common",
        "../../engine/includes",
        "../../engine/includes/resources",
        "../../engine/includes/components",
        "../../engine/includes/editor",
        "../../thirdparty/next/inc",
        "../../thirdparty/next/inc/math",
        "../../thirdparty/next/inc/core"
    ]

    DynamicLibrary {
        name: "gui-editor"
        condition: gui.desktop
        files: gui.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: {
            var result = gui.defines
            result.push("SHARED_DEFINE")
            result.push("GUI_LIBRARY")
            return result
        }
        cpp.includePaths: gui.incPaths
        cpp.cxxLanguageVersion: gui.languageVersion
        cpp.cxxStandardLibrary: gui.standardLibrary
        cpp.minimumMacosVersion: gui.osxVersion

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic gui"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: gui.PLUGINS_PATH
            qbs.installPrefix: gui.PREFIX
        }

        Group {
            name: "Module includes"
            files: [
                "includes/gui.h"
            ]
            qbs.install: true
            qbs.installDir: gui.INC_PATH + "/modules"
            qbs.installPrefix: gui.PREFIX
        }

        Group {
            name: "Install Dynamic gui"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: gui.PLUGINS_PATH
            qbs.installPrefix: gui.PREFIX
        }

        Group {
            name: "Module includes"
            prefix: "includes/"
            files: [
                "components/*.h"
            ]
            qbs.install: true
            qbs.installDir: gui.INC_PATH + "/engine"
            qbs.installPrefix: gui.PREFIX
        }
    }

    StaticLibrary {
        name: "gui"
        files: gui.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: gui.incPaths
        cpp.cxxLanguageVersion: gui.languageVersion
        cpp.cxxStandardLibrary: gui.standardLibrary
        cpp.minimumMacosVersion: gui.osxVersion
        cpp.minimumIosVersion: gui.iosVersion
        cpp.minimumTvosVersion: gui.tvosVersion

        Properties {
            condition: !gui.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: gui.ANDROID_STL
            Android.ndk.platform: gui.ANDROID
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
        }

        Group {
            name: "Install Static gui"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: gui.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: gui.PREFIX
        }
    }
}
