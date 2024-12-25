import qbs

Project {
    id: particletools
    property stringList srcFiles: [
        "*.cpp",
        "converter/**/*.cpp",
        "editor/**/*.cpp",
        "editor/**/*.ui",
        "property/*.cpp",
        "property/*.ui",
        "*.qrc",
        "*.h",
        "converter/**/*.h",
        "editor/**/*.h",
        "property/*.h"
    ]

    property stringList incPaths: [
        "converter",
        "property",
        "../../../engine/includes",
        "../../../engine/includes/resources",
        "../../../engine/includes/components",
        "../../../engine/includes/editor",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../modules/editor/grapheditor",
        "../../../modules/uikit/includes"
    ]

    DynamicLibrary {
        name: "particletools"
        condition: particletools.desktop
        files: particletools.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "graph-editor" }
        Depends { name: "uikit-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets", "xml"]; }
        bundle.isBundle: false

        cpp.defines: {
            var result  = particletools.defines
            result.push("SHARED_DEFINE")
            return result
        }
        cpp.includePaths: particletools.incPaths
        cpp.cxxLanguageVersion: particletools.languageVersion
        cpp.cxxStandardLibrary: particletools.standardLibrary
        cpp.minimumMacosVersion: particletools.osxVersion

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
            qbs.installDir: particletools.PLUGINS_PATH
            qbs.installPrefix: particletools.PREFIX
        }
    }
}
