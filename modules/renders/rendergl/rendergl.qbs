import qbs

Project {
    id: rendergl
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/resources/*.cpp",
    ]

    property stringList incPaths: [
        "includes",
        "../../../common",
        "../../../engine/includes/resources",
        "../../../engine/includes",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/glfw/include",
        "../../../thirdparty/glfm/include",
        "../../../thirdparty/glew/include",
        "../../../thirdparty/glad/include",
    ]

    DynamicLibrary {
        name: "rendergl-editor"
        condition: rendergl.desktop
        files: {
            var sources = srcFiles
            sources.push("src/editor/*.cpp")
            sources.push("includes/editor/*.h")
            return sources
        }
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "glfw-editor" }
        Depends { name: "glad" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: rendergl.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["OpenGL"]
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic RenderGL"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: rendergl.PLUGINS_PATH
            qbs.installPrefix: rendergl.PREFIX
        }

        Group {
            name: "RenderGL includes"
            prefix: "includes/"
            files: [
                "rendergl.h"
            ]
            qbs.install: true
            qbs.installDir: rendergl.INC_PATH + "/modules"
            qbs.installPrefix: rendergl.PREFIX
        }
    }

    StaticLibrary {
        name: "rendergl"
        files: rendergl.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: rendergl.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"
        cpp.cxxStandardLibrary: "libc++"
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: !rendergl.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: rendergl.ANDROID_STL
            Android.ndk.platform: rendergl.ANDROID
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["OpenGL"]
        }

        Group {
            name: "Install Static RenderGL"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: rendergl.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: rendergl.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: qbs.buildVariant === "release" ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: rendergl.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: rendergl.PREFIX
        }
    }
}
