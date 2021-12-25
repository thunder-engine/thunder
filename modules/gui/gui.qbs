import qbs

Project {
    id: gui
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/systems/*.cpp",

        "includes/components/*.h",
        "includes/systems/*.h"
    ]

    property stringList incPaths: [
        "includes",
        "../../common",
        "../../engine/includes",
        "../../engine/includes/resources",
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

        cpp.defines: ["NEXT_SHARED"]
        cpp.includePaths: gui.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"

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
            name: "Gui includes"
            prefix: "includes/"
            files: [
                "gui.h"
            ]
            qbs.install: true
            qbs.installDir: gui.INC_PATH + "/modules"
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
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"
        cpp.cxxStandardLibrary: "libc++"

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
