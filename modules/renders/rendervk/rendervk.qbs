import qbs
import qbs.Environment

Project {
    id: rendervk
    condition: rendervk.withVulkan

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
        "../../../engine/includes/components",
        "../../../engine/includes/resources",
        "../../../engine/includes",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/glfw/include",
        "../../../thirdparty/glfm/include",
        "../../../thirdparty/vulkan/include",
        Environment.getEnv("VULKAN_SDK") + "/include"
    ]

    DynamicLibrary {
        name: "rendervk-editor"
        condition: rendervk.desktop
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
        Depends { name: "Qt"; submodules: ["core", "widgets", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: rendervk.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.libraryPaths: [Environment.getEnv("VULKAN_SDK") + "/Lib"]
            cpp.staticLibraries: [ "vulkan-1" ]
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic rendervk"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: rendervk.PLUGINS_PATH
            qbs.installPrefix: rendervk.PREFIX
        }
    }

    StaticLibrary {
        name: "rendervk"
        condition: rendervk.desktop
        files: rendervk.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: rendervk.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"
        cpp.cxxStandardLibrary: "libc++"
        cpp.debugInformation: qbs.buildVariant === "release"
        cpp.separateDebugInformation: cpp.debugInformation

        Properties {
            condition: !rendervk.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: rendervk.ANDROID_STL
            Android.ndk.platform: rendervk.ANDROID
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
        }

        Group {
            name: "Install Static rendervk"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: rendervk.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: rendervk.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: cpp.debugInformation ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: rendervk.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: rendervk.PREFIX
        }
    }
}
