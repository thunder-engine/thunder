import qbs

Project {
    id: navigation
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/resources/*.cpp",
        "src/utils/*.cpp"
    ]

    property stringList incPaths: [
        "editor",
        "includes",
        "../physics/bullet/includes",
        "../physics/bullet/includes/components",
        "../physics/bullet/includes/resources",
        "../../engine/includes",
        "../../engine/includes/components",
        "../../engine/includes/resources",
        "../../editor/includes",
        "../../thirdparty/next/inc",
        "../../thirdparty/next/inc/math",
        "../../thirdparty/next/inc/core",
        "../../thirdparty/recast/Recast/Include",
        "../../thirdparty/recast/Detour/Include",
        "../../thirdparty/recast/DetourCrowd/Include",
        "../../thirdparty/recast/DetourTileCache/Include",
        "../../thirdparty/recast/DebugUtils/Include",
    ]

    DynamicLibrary {
        name: "navigation-editor"
        condition: navigation.desktop
        files: [
            "editor/*.cpp",
            "editor/*.h",
            "editor/*.ui",
            "editor/property/*.cpp",
            "editor/property/*.h",
            "editor/property/*.ui"
        ].concat(navigation.srcFiles)
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "bullet-editor" }
        Depends { name: "editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }
        Depends { name: "recast" }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE", "NAVIGATION_LIBRARY"]
        cpp.includePaths: navigation.incPaths
        cpp.staticLibraries: [ ]
        cpp.cxxLanguageVersion: navigation.languageVersion
        cpp.cxxStandardLibrary: navigation.standardLibrary
        cpp.minimumMacosVersion: navigation.osxVersion

        Group {
            name: "Install Dynamic navigation"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: navigation.PLUGINS_PATH
            qbs.installPrefix: navigation.PREFIX
        }
    }

    StaticLibrary {
        name: "navigation"
        files: srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: navigation.incPaths
        cpp.cxxLanguageVersion: navigation.languageVersion
        cpp.cxxStandardLibrary: navigation.standardLibrary
        cpp.minimumMacosVersion: navigation.osxVersion
        cpp.minimumIosVersion: navigation.iosVersion
        cpp.minimumTvosVersion: navigation.tvosVersion
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: !navigation.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: navigation.ANDROID_STL
            Android.ndk.platform: navigation.ANDROID
        }

        Group {
            name: "Module includes"
            files: [
                "includes/navigation.h"
            ]
            qbs.install: true
            qbs.installDir: navigation.INC_PATH + "/modules"
            qbs.installPrefix: navigation.PREFIX
        }

        Group {
            name: "Engine includes"
            prefix: "includes/"
            files: [
                "components/*.h",
                "resources/*.h"
            ]
            qbs.install: true
            qbs.installDir: navigation.INC_PATH + "/engine"
            qbs.installPrefix: navigation.PREFIX
        }

        Group {
            name: "Install Static Navigation"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: navigation.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: navigation.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: cpp.debugInformation ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: navigation.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: navigation.PREFIX
        }
    }
}
