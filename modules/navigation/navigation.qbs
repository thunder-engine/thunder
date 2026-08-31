import qbs

Project {
    id: navigation
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/resources/*.cpp"
    ]

    property stringList incPaths: [
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
    ]

    DynamicLibrary {
        name: "navigation-editor"
        condition: navigation.desktop
        files: srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "bullet-editor" }
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
        files: navigation.srcFiles
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
            name: "Install Static navigation"
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
