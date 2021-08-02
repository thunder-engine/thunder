import qbs

Project {
    id: next
    property stringList srcFiles: [
        "src/*.cpp",
        "src/core/*.cpp",
        "src/math/*.cpp",
        "src/anim/*.cpp",
        "src/analytics/*.cpp",
        "inc/*.h",
        "inc/core/*.h",
        "inc/math/*.h",
        "inc/anim/*.h",
        "inc/analytics/*.h"
    ]

    property stringList incPaths: [
        "inc",
        "inc/core",
        "inc/math",
        "inc/anim",
        "inc/analytics"
    ]

    DynamicLibrary {
        name: "next-editor"
        condition: next.desktop
        files: next.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: ["NEXT_SHARED", "NEXT_LIBRARY"]
        cpp.includePaths: next.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic Platform"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: next.LIB_PATH + "/" + next.bundle
            qbs.installPrefix: next.PREFIX
        }

    }

    StaticLibrary {
        name: "next"
        files: next.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: ["NEXT_LIBRARY"]
        cpp.includePaths: next.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"
        cpp.cxxStandardLibrary: "libc++"
        cpp.debugInformation: qbs.buildVariant === "release"
        cpp.separateDebugInformation: cpp.debugInformation

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: next.ANDROID_STL
            Android.ndk.platform: next.ANDROID
        }

        Group {
            name: "Install Static Platform"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: next.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: next.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: cpp.debugInformation ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: next.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: next.PREFIX
        }
    }
}
