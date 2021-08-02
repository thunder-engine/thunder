import qbs

Project {
    id: engine
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/**/*.cpp",
        "src/resources/*.cpp",
        "src/systems/*.cpp",
        "src/filters/*.cpp",
        "src/postprocess/*.cpp",
        "includes/*.h",
        "includes/adapters/*.h",
        "includes/components/**/*.h",
        "includes/resources/*.h",
        "includes/systems/*.h",
        "includes/filters/*.h",
        "includes/postprocess/*.h"
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
        files:
        {
            var sources = srcFiles
            sources.push("src/editor/*.cpp")
            sources.push("includes/editor/*.h")
            return sources
        }
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "glfw-editor" }
        Depends { name: "zlib-editor" }
        Depends { name: "physfs-editor" }
        Depends { name: "freetype-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["NEXT_SHARED", "NEXT_LIBRARY"]
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
        cpp.debugInformation: qbs.buildVariant === "release"
        cpp.separateDebugInformation: cpp.debugInformation

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
            fileTagsFilter: cpp.debugInformation ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: engine.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: engine.PREFIX
        }
    }
}
