import qbs

Project {
    id: engine
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/**/*.cpp",
        "src/resources/*.cpp",
        "src/systems/*.cpp",
        "src/filters/*.cpp",
        "src/postprocess/*.cpp"
    ]

    property stringList incPaths: [
        "../",
        "../thirdparty/next/inc",
        "../thirdparty/next/inc/math",
        "../thirdparty/next/inc/core",
        "../thirdparty/next/inc/anim",
        "../thirdparty/physfs/src",
        "../thirdparty/glfw/include",
        "../thirdparty/glfm/include",
        "../thirdparty/freetype/include",
        "includes/components",
        "includes/resources",
        "includes/adapters",
        "includes/editor",
        "includes"
    ]

    DynamicLibrary {
        name: "engine-editor"
        condition: engine.desktop
        files: {
            var sources = srcFiles
            sources.push("src/editor/**/*.cpp")
            return sources
        }
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "glfw-editor" }
        Depends { name: "zlib-editor" }
        Depends { name: "physfs-editor" }
        Depends { name: "freetype-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }
        bundle.isBundle: false

        cpp.defines: {
            var result = engine.defines
            result.push("NEXT_SHARED")
            result.push("NEXT_LIBRARY")
            return result
        }
        cpp.includePaths: engine.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"

        Properties {
            condition: engine.desktop
            files: outer.concat(["src/adapters/desktopadaptor.cpp"])
        }

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.dynamicLibraries: [ "Shell32" ]
        }

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["IOKit"]
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic Engine"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: engine.LIB_PATH + "/" + engine.bundle
            qbs.installPrefix: engine.PREFIX
        }

        Group {
            name: "Engine includes"
            prefix: "includes/"
            files: [
                "*.h",
                "components/*.h",
                "resources/*.h",
                "postprocess/postprocessor.h",
            ]
            qbs.install: true
            qbs.installDir: engine.INC_PATH + "/engine"
            qbs.installPrefix: engine.PREFIX
        }

        Group {
            name: "Editor includes"
            prefix: "includes/editor"
            files: [
                "**/*.h"
            ]
            qbs.install: true
            qbs.installDir: engine.INC_PATH + "/editor"
            qbs.installPrefix: engine.PREFIX
        }

        Group {
            name: "Viewport Editor includes"
            prefix: "includes/editor/viewport"
            files: [
                "**/*.h"
            ]
            qbs.install: true
            qbs.installDir: engine.INC_PATH + "/editor/viewport"
            qbs.installPrefix: engine.PREFIX
        }
    }

    StaticLibrary {
        name: "engine"
        files: engine.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next" }
        bundle.isBundle: false

        cpp.includePaths: engine.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"
        cpp.cxxStandardLibrary: "libc++"
        cpp.defines: ["NEXT_LIBRARY"]
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"

        Properties {
            condition: engine.desktop
            files: outer.concat(["src/adapters/desktopadaptor.cpp"])
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            files: outer.concat(["src/adapters/mobileadaptor.cpp"])
            cpp.defines: ["THUNDER_MOBILE"]
            Android.ndk.appStl: engine.ANDROID_STL
            Android.ndk.platform: engine.ANDROID
        }

        Properties {
            condition: qbs.targetOS.contains("ios")
            files: outer.concat(["src/adapters/mobileadaptor.cpp", "src/adapters/appleplatform.mm"])
            cpp.defines: ["THUNDER_MOBILE", "TARGET_OS_IOS"]
        }

        Properties {
            condition: qbs.targetOS.contains("tvos")
            files: outer.concat(["src/adapters/mobileadaptor.cpp", "src/adapters/appleplatform.mm"])
            cpp.defines: ["THUNDER_MOBILE", "TARGET_OS_TV"]
        }

        Group {
            name: "Install Static Engine"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: engine.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: engine.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: qbs.buildVariant === "release" ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: engine.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: engine.PREFIX
        }
    }
}
