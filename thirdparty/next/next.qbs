import qbs

Project {
    id: next
    property stringList srcFiles: [
        "src/*.cpp",
        "src/core/*.cpp",
        "src/math/*.cpp",
        "src/anim/*.cpp",
        "src/analytics/*.cpp"
    ]

    property stringList incPaths: [
        "inc",
        "inc/core",
        "inc/math",
        "inc/anim",
        "inc/analytics"
    ]

    DynamicLibrary {
        name: "next-editor"
        condition: next.desktop
        files: next.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE", "NEXT_LIBRARY"]
        cpp.includePaths: next.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]
        cpp.cxxLanguageVersion: next.languageVersion
        cpp.cxxStandardLibrary: next.standardLibrary
        cpp.minimumMacosVersion: next.osxVersion

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic Platform"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: next.LIB_PATH
            qbs.installPrefix: next.PREFIX
        }

        Group {
            name: "Next includes"
            prefix: "inc/"
            files: [
                "**"
            ]
            qbs.install: true
            qbs.installDir: next.INC_PATH + "/next"
            qbs.installPrefix: next.PREFIX
            qbs.installSourceBase: prefix
        }
    }

    StaticLibrary {
        name: "next"
        files: next.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: ["NEXT_LIBRARY"]
        cpp.includePaths: next.incPaths
        cpp.cxxLanguageVersion: next.languageVersion
        cpp.cxxStandardLibrary: next.standardLibrary
        cpp.minimumMacosVersion: next.osxVersion
        cpp.minimumIosVersion: next.iosVersion
        cpp.minimumTvosVersion: next.tvosVersion
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: next.ANDROID_STL
            Android.ndk.platform: next.ANDROID
        }

        Group {
            name: "Install Static Platform"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: next.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: next.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: qbs.buildVariant === "release" ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: next.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: next.PREFIX
        }
    }
}
