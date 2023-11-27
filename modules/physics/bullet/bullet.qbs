import qbs

Project {
    id: bullet
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/resources/*.cpp"
    ]

    property stringList incPaths: [
        "includes",
        "includes/resources",
        "../../../common",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../engine/includes",
        "../../../engine/includes/components",
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
            sources.push("includes/converters/*.h")
            sources.push("src/converters/templates.qrc")
            return sources
        }
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "bullet3" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE", "BULLET_LIBRARY"]
        cpp.includePaths: bullet.incPaths
        cpp.cxxLanguageVersion: bullet.languageVersion
        cpp.cxxStandardLibrary: bullet.standardLibrary
        cpp.minimumMacosVersion: bullet.osxVersion

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
			cpp.commonCompilerFlags: "-Wno-argument-outside-range"
        }

        Group {
            name: "Install Dynamic bullet"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: bullet.PLUGINS_PATH
            qbs.installPrefix: bullet.PREFIX
        }

        Group {
            name: "Module includes"
            files: [
                "includes/bullet.h"
            ]
            qbs.install: true
            qbs.installDir: bullet.INC_PATH + "/modules"
            qbs.installPrefix: bullet.PREFIX
        }

        Group {
            name: "Engine includes"
            prefix: "includes/"
            files: [
                "components/*.h",
                "resources/*.h"
            ]
            qbs.install: true
            qbs.installDir: bullet.INC_PATH + "/engine"
            qbs.installPrefix: bullet.PREFIX
        }
    }

    StaticLibrary {
        name: "bullet"
        files: srcFiles
        Depends { name: "cpp" }
        Depends { name: "bullet3" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: bullet.incPaths
        cpp.cxxLanguageVersion: bullet.languageVersion
        cpp.cxxStandardLibrary: bullet.standardLibrary
        cpp.minimumMacosVersion: bullet.osxVersion
        cpp.minimumIosVersion: bullet.iosVersion
        cpp.minimumTvosVersion: bullet.tvosVersion
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: !bullet.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("windows") && !qbs.toolchain.contains("gcc")
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: bullet.ANDROID_STL
            Android.ndk.platform: bullet.ANDROID
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
			cpp.commonCompilerFlags: "-Wno-argument-outside-range"
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
            fileTagsFilter: qbs.buildVariant === "release" ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: bullet.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: bullet.PREFIX
        }
    }
}
