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
            name: "Bullet includes"
            prefix: "includes/"
            files: [
                "bullet.h"
            ]
            qbs.install: true
            qbs.installDir: bullet.INC_PATH + "/modules"
            qbs.installPrefix: bullet.PREFIX
        }
    }

    StaticLibrary {
        name: "bullet"
        files: [
            "src/converters/templates.qrc",
        ].concat(bullet.srcFiles)
        Depends { name: "cpp" }
        Depends { name: "bullet3" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: bullet.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"
        cpp.cxxStandardLibrary: "libc++"
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        cpp.linkerFlags: ["--whole-archive -lbullet3 --no-whole-archive"]

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
