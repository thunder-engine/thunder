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
        "../../../thirdparty/glfw/include",
        "../../../thirdparty/glew/include",
        "../../../thirdparty/glad/include",
    ]

    DynamicLibrary {
        name: "rendergl-editor"
        condition: rendergl.desktop
        files: rendergl.srcFiles
        Depends { name: "cpp" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "glfw-editor" }
        Depends { name: "glad" }
        bundle.isBundle: false

        cpp.defines: ["BUILD_SHARED", "NEXT_LIBRARY"]
        cpp.includePaths: rendergl.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.sonamePrefix: "@executable_path"

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.libraryPaths: [
                //"../../../thirdparty/glew/bin"
            ]
            //cpp.dynamicLibraries: outer.concat([
            //    "glew32",
            //    "opengl32"
            //])
        }

        Properties {
            condition: qbs.targetOS.contains("osx")
            cpp.weakFrameworks: ["OpenGL"]
        }

        Group {
            name: "Install Dynamic RenderGL"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: rendergl.BIN_PATH + "/" + rendergl.bundle
            qbs.installPrefix: rendergl.PREFIX
        }
    }

    StaticLibrary {
        name: "rendergl"
        files: rendergl.srcFiles
        Depends { name: "cpp" }
        bundle.isBundle: false

        cpp.includePaths: rendergl.incPaths
        cpp.cxxLanguageVersion: "c++14"

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: "gnustl_shared"
        }

        Properties {
            condition: qbs.targetOS.contains("osx")
            cpp.weakFrameworks: ["OpenGL"]
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
