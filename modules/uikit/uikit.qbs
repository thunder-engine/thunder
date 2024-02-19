import qbs

Project {
    id: uikit
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/pipelinetasks/*.cpp",
        "src/resources/*.cpp",
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
        "../../thirdparty/next/inc/core",
        "../../thirdparty/openal/include",
        "../../thirdparty/libogg/src",
        "../../thirdparty/libvorbis/src"
    ]

    DynamicLibrary {
        name: "uikit-editor"
        condition: uikit.desktop
        files: {
            var sources = srcFiles
            sources.push("src/converters/*.cpp")
            sources.push("includes/converters/*.h")
            return sources
        }
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE", "ENGINE_LIBRARY"]
        cpp.includePaths: uikit.incPaths
        cpp.cxxLanguageVersion: uikit.languageVersion
        cpp.cxxStandardLibrary: uikit.standardLibrary
        cpp.minimumMacosVersion: uikit.osxVersion

        Group {
            name: "Install Dynamic uikit"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: uikit.LIB_PATH
            qbs.installPrefix: uikit.PREFIX
        }

        Group {
            name: "Module includes"
            files: [
                "includes/uikit.h"
            ]
            qbs.install: true
            qbs.installDir: uikit.INC_PATH + "/modules"
            qbs.installPrefix: uikit.PREFIX
        }

        Group {
            name: "Engine includes"
            prefix: "includes/"
            files: [
                "components/*.h",
                "resources/*.h"
            ]
            qbs.install: true
            qbs.installDir: uikit.INC_PATH + "/engine"
            qbs.installPrefix: uikit.PREFIX
        }
    }

    StaticLibrary {
        name: "uikit"
        files: uikit.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: uikit.incPaths
        cpp.cxxLanguageVersion: uikit.languageVersion
        cpp.cxxStandardLibrary: uikit.standardLibrary
        cpp.minimumMacosVersion: uikit.osxVersion
        cpp.minimumIosVersion: uikit.iosVersion
        cpp.minimumTvosVersion: uikit.tvosVersion
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: !uikit.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: uikit.ANDROID_STL
            Android.ndk.platform: uikit.ANDROID
        }

        Group {
            name: "Install Static UiKit"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: uikit.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: uikit.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: qbs.buildVariant === "release" ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: uikit.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: uikit.PREFIX
        }
    }
}
