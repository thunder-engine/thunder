import qbs

Project {
    id: recast
    property stringList srcFiles: [
        "Detour/Source/*.cpp",
        "DetourCrowd/Source/*.cpp",
        "DetourTileCache/Source/*.cpp",
        "Recast/Source/*.cpp"
    ]

    property stringList incPaths: [
        "DebugUtils/Include",
        "Detour/Include",
        "DetourCrowd/Include",
        "DetourTileCache/Include",
        "Recast/Include"
    ]

    StaticLibrary {
        name: "recast"
        files: recast.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: []
        cpp.includePaths: recast.incPaths
        cpp.cxxLanguageVersion: recast.languageVersion
        cpp.cxxStandardLibrary: recast.standardLibrary
        cpp.minimumMacosVersion: recast.osxVersion
        cpp.minimumIosVersion: recast.iosVersion
        cpp.minimumTvosVersion: recast.tvosVersion
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: recast.desktop
            files: outer.concat([
                "DebugUtils/Source/*.cpp"
            ])
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: recast.ANDROID_STL
            Android.ndk.platform: recast.ANDROID
        }

        Group {
            name: "Install Static Platform"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: recast.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: recast.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: qbs.buildVariant === "release" ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: recast.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: recast.PREFIX
        }
    }
}
