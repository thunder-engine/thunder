import qbs

Project {
    id: media
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
        "../../common",
        "../../engine/includes",
        "../../engine/includes/resources",
        "../../engine/includes/converters",
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
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "ogg-editor" }
        Depends { name: "vorbis-editor" }
        Depends { name: "vorbisfile-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "multimedia"]; }
        bundle.isBundle: false

        cpp.defines: ["BUILD_SHARED", "NEXT_LIBRARY"]
        cpp.includePaths: media.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"
        cpp.sonamePrefix: "@executable_path"

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.libraryPaths: [ "../../thirdparty/openal/windows/x32" ]
            cpp.dynamicLibraries: [ "OpenAL32" ]
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["OpenAL"]
        }

        Group {
            name: "Install Dynamic media"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: media.BIN_PATH + "/plugins/" + media.bundle
            qbs.installPrefix: media.PREFIX
        }
    }

    StaticLibrary {
        name: "media"
        files: media.srcFiles
        Depends { name: "cpp" }
        bundle.isBundle: false

        cpp.includePaths: media.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"
        cpp.cxxStandardLibrary: "libc++"

        Properties {
            condition: !media.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: "gnustl_shared"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["OpenAL"]
        }

        Group {
            name: "Install Static media"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: media.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/lib"
            qbs.installPrefix: media.PREFIX
        }
    }
}
