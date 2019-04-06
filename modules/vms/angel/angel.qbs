import qbs

Project {
    id: angel
    property stringList srcFiles: [
        "src/*.cpp",
        "src/bindings/*.cpp",
        "src/components/*.cpp",
        "src/resources/*.cpp",

        "includes/*.h",
        "includes/bindings/*.h",
        "includes/components/*.h",
        "includes/resources/*.h",
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
        "../../../engine/includes/converters",
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
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "angelscript-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["NEXT_SHARED"]
        cpp.includePaths: angel.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"
        cpp.sonamePrefix: "@executable_path"

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
        }

        Group {
            name: "Install Dynamic angel"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: angel.PLUGINS_PATH
            qbs.installPrefix: angel.PREFIX
        }
    }

    StaticLibrary {
        name: "angel"
        files: angel.srcFiles
        Depends { name: "cpp" }
        bundle.isBundle: false

        cpp.includePaths: angel.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"
        cpp.cxxStandardLibrary: "libc++"

        Properties {
            condition: !angel.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: "gnustl_static"
            Android.ndk.platform: angel.ANDROID
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["OpenAL"]
        }

        Group {
            name: "Install Static angel"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: angel.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/lib"
            qbs.installPrefix: angel.PREFIX
        }
    }
}
