import qbs

Project {
    id: pipelinetools
    property stringList srcFiles: [
        "*.cpp",
        "converter/*.cpp",
        "editor/**/*.cpp",
        "editor/**/*.ui",
        "*.qrc",
        "*.h",
        "converter/**/*.h",
        "editor/**/*.h"
    ]

    property stringList incPaths: [
        "../../../engine/includes",
        "../../../engine/includes/resources",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../modules/editor/grapheditor"
    ]

    DynamicLibrary {
        name: "pipelinetools"
        condition: pipelinetools.desktop
        files: pipelinetools.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "graph-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets", "xml"]; }
        bundle.isBundle: false

        cpp.defines: {
            var result  = pipelinetools.defines
            result.push("SHARED_DEFINE")
            return result
        }
        cpp.includePaths: pipelinetools.incPaths
        cpp.cxxLanguageVersion: pipelinetools.languageVersion
        cpp.cxxStandardLibrary: pipelinetools.standardLibrary
        cpp.minimumMacosVersion: pipelinetools.osxVersion

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
            qbs.installDir: pipelinetools.PLUGINS_PATH
            qbs.installPrefix: pipelinetools.PREFIX
        }
    }
}
