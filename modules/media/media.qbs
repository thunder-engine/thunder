import qbs

Project {
    id: media
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
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
        name: "media-editor"
        condition: media.desktop
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
        Depends { name: "ogg-editor" }
        Depends { name: "vorbis-editor" }
        Depends { name: "vorbisfile-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "multimedia"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE", "MEDIA_LIBRARY"]
        cpp.includePaths: media.incPaths
        cpp.cxxLanguageVersion: media.languageVersion
        cpp.cxxStandardLibrary: media.standardLibrary
        cpp.minimumMacosVersion: media.osxVersion

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.libraryPaths: ["../../thirdparty/openal/windows/" + ((qbs.architecture === "x86_64") ? "x64" : "x32")]
            cpp.dynamicLibraries: ["OpenAL32"]
        }

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../../lib"
            cpp.dynamicLibraries: ["openal"]
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["OpenAL"]
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic media"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: media.PLUGINS_PATH
            qbs.installPrefix: media.PREFIX
        }

        Group {
            name: "Module includes"
            files: [
                "includes/media.h"
            ]
            qbs.install: true
            qbs.installDir: media.INC_PATH + "/modules"
            qbs.installPrefix: media.PREFIX
        }

        Group {
            name: "Engine includes"
            prefix: "includes/"
            files: [
                "components/*.h",
                "resources/*.h"
            ]
            qbs.install: true
            qbs.installDir: media.INC_PATH + "/engine"
            qbs.installPrefix: media.PREFIX
        }

        Group {
            name: "OpenAL Binary"
            condition: qbs.targetOS.contains("windows")
            files: [
                "../../thirdparty/openal/windows/bin/OpenAL32.dll"
            ]
            qbs.install: true
            qbs.installDir: media.BIN_PATH + "/" + media.bundle
            qbs.installPrefix: media.PREFIX
        }
    }

    StaticLibrary {
        name: "media"
        files: media.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: media.incPaths
        cpp.cxxLanguageVersion: media.languageVersion
        cpp.cxxStandardLibrary: media.standardLibrary
        cpp.minimumMacosVersion: media.osxVersion
        cpp.minimumIosVersion: media.iosVersion
        cpp.minimumTvosVersion: media.tvosVersion
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: !media.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: media.ANDROID_STL
            Android.ndk.platform: media.ANDROID
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["OpenAL"]
        }

        Group {
            name: "Install Static media"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: media.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: media.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: qbs.buildVariant === "release" ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: media.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: media.PREFIX
        }
    }
}
