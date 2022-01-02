import qbs

Project {
    id: shadertools
    property stringList srcFiles: [
        "*.cpp",
        "converter/*.cpp",
        "editor/**/*.cpp",
        "editor/**/*.ui",

        "*.h",
        "converter/**/*.h",
        "editor/**/*.h"
    ]

    property stringList incPaths: [
        "../../../engine/includes",
        "../../../engine/includes/resources",
        "../../../engine/includes/editor",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/spirvcross/src",
        "../../../thirdparty/glsl"
    ]

    DynamicLibrary {
        name: "shadertools"
        condition: shadertools.desktop
        files: shadertools.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "glsl" }
        Depends { name: "spirvcross" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets", "quickwidgets"]; }
        bundle.isBundle: false

        cpp.defines: ["NEXT_SHARED"]
        cpp.includePaths: shadertools.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Plugin"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: shadertools.PLUGINS_PATH
            qbs.installPrefix: shadertools.PREFIX
        }
    }
}
