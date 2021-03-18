import qbs

Project {
    id: bullet
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/resources/*.cpp",

        "includes/*.h",
        "includes/components/*.h",
        "includes/resources/*.h"
    ]

    property stringList incPaths: [
        "includes",
        "../../../common",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../engine/includes",
        "../../../engine/includes/resources",
        "../../../engine/includes/editor",
        "../../../thirdparty/bullet/src"
    ]

    DynamicLibrary {
        name: "bullet-editor"
        condition: bullet.desktop
        files: {
            var sources = srcFiles
            sources.push("src/converters/*.cpp")
            sources.push("src/converters/*.qrc")
            sources.push("includes/converters/*.h")
            return sources
        }
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "bullet3" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["NEXT_SHARED"]
        cpp.includePaths: bullet.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic bullet"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: bullet.PLUGINS_PATH
            qbs.installPrefix: bullet.PREFIX
        }
    }

    StaticLibrary {
        name: "bullet"
        files: [
            "src/converters/templates.qrc",
        ].concat(bullet.srcFiles)
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: bullet.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"
        cpp.cxxStandardLibrary: "libc++"
        cpp.debugInformation: qbs.buildVariant === "release"
        cpp.separateDebugInformation: cpp.debugInformation

        Properties {
            condition: !bullet.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: "gnustl_static"
            Android.ndk.platform: bullet.ANDROID
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["OpenAL"]
        }

        Group {
            name: "Install Static bullet"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir:  bullet.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: bullet.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: cpp.debugInformation ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: bullet.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: bullet.PREFIX
        }
    }
}
