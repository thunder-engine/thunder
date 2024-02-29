import qbs

Project {
    id: uikit
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/pipelinetasks/*.cpp",
        "src/resources/*.cpp",
        "src/utils/*.cpp",
        "../../thirdparty/pugixml/src/*.cpp"
    ]

    property stringList incPaths: [
        "includes",
        "includes/resources",
        "../../common",
        "../../engine/includes",
        "../../engine/includes/resources",
        "../../engine/includes/components",
        "../../engine/includes/editor",
        "../../thirdparty/next/inc",
        "../../thirdparty/next/inc/math",
        "../../thirdparty/next/inc/core",
        "../../thirdparty/pugixml/src",
        "../../thirdparty/cssparser/include",
    ]

    DynamicLibrary {
        name: "uikit-editor"
        condition: uikit.desktop
        files: {
            var sources = srcFiles
            sources.push("src/converters/*.cpp")
            sources.push("includes/converters/*.h")
            sources.push("src/editor/**/*.cpp")
            sources.push("src/editor/**/*.h")
            sources.push("src/editor/**/*.ui")
            sources.push("src/editor/**/*.qrc")
            return sources
        }
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }
        bundle.isBundle: false

        cpp.defines: {
            var result = uikit.defines
            result.push("SHARED_DEFINE")
            result.push("UIKIT_LIBRARY")
            return result
        }
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
