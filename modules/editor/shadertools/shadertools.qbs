import qbs

Project {
    id: shadertools
    property stringList srcFiles: [
        "*.cpp",
        "converter/*.cpp",
        "editor/**/*.cpp",
        "editor/**/*.ui",
        "*.qrc",
        "*.h",
        "converter/**/*.h",
        "converter/**/*.cpp",
        "editor/**/*.h"
    ]

    property stringList incPaths: [
        "../../../engine/includes",
        "../../../engine/includes/resources",
        "../../../engine/includes/components",
        "../../../engine/includes/editor",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/spirvcross/src",
        "../../../thirdparty/pugixml/src",
        "../../../thirdparty/glsl",
        "../../../modules/editor/grapheditor",
        "../../../modules/uikit/includes"
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
        Depends { name: "graph-editor" }
        Depends { name: "uikit-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets", "xml"]; }
        bundle.isBundle: false

        cpp.defines: {
            var result  = shadertools.defines
            result.push("SHARED_DEFINE")
            return result
        }
        cpp.includePaths: shadertools.incPaths
        cpp.cxxLanguageVersion: shadertools.languageVersion
        cpp.cxxStandardLibrary: shadertools.standardLibrary
        cpp.minimumMacosVersion: shadertools.osxVersion

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
