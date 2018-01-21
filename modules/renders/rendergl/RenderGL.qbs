import qbs

Project {
    id: rendergl
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/postprocess/*.cpp",
        "src/resources/*.cpp",
        "src/filters/*.cpp",

        "includes/*.h",
        "includes/components/*.h",
        "includes/resources/*.h",
        "includes/postprocess/*.h",
        "includes/filters/*.h"
    ]

    property stringList incPaths: [
        "includes",
        "../../../common",
        "../../../engine/includes",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../../external/nvidia/inc"
    ]

    DynamicLibrary {
        name: "rendergl-editor"
        condition: rendergl.desktop
        files: rendergl.srcFiles
        Depends { name: "cpp" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }

        cpp.defines: ["BUILD_SHARED", "NEXT_LIBRARY"]
        cpp.includePaths: rendergl.incPaths
        cpp.libraryPaths: [
            "../../../../external/nvidia/lib"
        ]
        cpp.dynamicLibraries: [
            "glew32",
            "opengl32"
        ]

        Group {
            name: "Install Dynamic RenderGL"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: rendergl.BIN_PATH
            qbs.installPrefix: rendergl.PREFIX
        }
    }

    StaticLibrary {
        name: "rendergl"
        files: rendergl.srcFiles
        Depends { name: "cpp" }

        cpp.includePaths: rendergl.incPaths
        cpp.cxxLanguageVersion: "c++14"

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: "gnustl_shared"
        }

        Group {
            name: "Install Static RenderGL"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: rendergl.LIB_PATH
            qbs.installPrefix: rendergl.PREFIX
        }
    }
}
