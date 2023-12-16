import qbs

Project {
    id: angel
    property stringList srcFiles: [
        "src/*.cpp",
        "src/bindings/*.cpp",
        "src/components/*.cpp",
        "src/resources/*.cpp",

        "../../../thirdparty/angelscript/modules/*/*.cpp"
    ]

    property stringList incPaths: [
        "includes",
        "../../../common",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../engine/includes",
        "../../../engine/includes/resources",
        "../../../engine/includes/components",
        "../../../engine/includes/editor",
        "../../../thirdparty/angelscript/include",
        "../../../thirdparty/angelscript/modules"
    ]

    DynamicLibrary {
        name: "angel-editor"
        condition: angel.desktop
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
        Depends { name: "angelscript-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: angel.incPaths
        cpp.cxxLanguageVersion: angel.languageVersion
        cpp.cxxStandardLibrary: angel.standardLibrary
        cpp.minimumMacosVersion: angel.osxVersion

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic angel"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: angel.PLUGINS_PATH
            qbs.installPrefix: angel.PREFIX
        }

        Group {
            name: "Angel includes"
            prefix: "includes/"
            files: [
                "angel.h"
            ]
            qbs.install: true
            qbs.installDir: angel.INC_PATH + "/modules"
            qbs.installPrefix: angel.PREFIX
        }
    }

    StaticLibrary {
        name: "angel"
        files: angel.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: angel.incPaths
        cpp.cxxLanguageVersion: angel.languageVersion
        cpp.cxxStandardLibrary: angel.standardLibrary
        cpp.minimumMacosVersion: angel.osxVersion
        cpp.minimumIosVersion: angel.iosVersion
        cpp.minimumTvosVersion: angel.tvosVersion
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: !angel.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: angel.ANDROID_STL
            Android.ndk.platform: angel.ANDROID
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
        }

        Group {
            name: "Install Static angel"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: angel.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: angel.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: qbs.buildVariant === "release" ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: angel.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: angel.PREFIX
        }
    }
}
