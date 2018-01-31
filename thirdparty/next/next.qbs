import qbs

Project {
    id: next
    property stringList srcFiles: [
        "src/*.cpp",
        "src/core/*.cpp",
        "src/math/*.cpp",
        "inc/core/*.h",
        "inc/math/*.h"
    ]

    property stringList incPaths: [
        "inc",
        "inc/core",
        "inc/math"
    ]

    DynamicLibrary {
        name: "next-editor"
        condition: next.desktop
        files: next.srcFiles
        Depends { name: "cpp" }
        bundle.isBundle: false

        cpp.defines: ["BUILD_SHARED", "NEXT_LIBRARY"]
        cpp.includePaths: next.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]
        cpp.cxxLanguageVersion: "c++14"
        cpp.sonamePrefix: "@executable_path"

        Group {
            name: "Install Dynamic Platform"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: next.BIN_PATH + "/" + next.bundle
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

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: "gnustl_shared"
        }

        Group {
            name: "Install Static Platform"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: next.LIB_PATH
            qbs.installPrefix: next.PREFIX
        }
    }
}
