import qbs

Project {
    id: grapheditor
    property stringList srcFiles: [
        "*.cpp",
        "editor/graph/**/*.cpp",
        "editor/graph/actions/*.h",
        "editor/graph/graphwidgets/*.h",
        "*.h"
    ]

    property stringList incPaths: [
        "./",
        "../../../engine/includes",
        "../../../engine/includes/components",
        "../../../engine/includes/resources",
        "../../../engine/includes/editor",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/pugixml/src",
        "../../../modules/uikit/includes/",
        "../../../modules/uikit/includes/components"
    ]

    DynamicLibrary {
        name: "graph-editor"
        condition: grapheditor.desktop
        files: grapheditor.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "uikit-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets", "xml"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE", "NODEGRAPH_LIBRARY"]
        cpp.includePaths: grapheditor.incPaths
        cpp.cxxLanguageVersion: grapheditor.languageVersion
        cpp.cxxStandardLibrary: grapheditor.standardLibrary
        cpp.minimumMacosVersion: grapheditor.osxVersion

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Graph Editor includes"
            prefix: "editor/graph"
            files: [
                "**/*.h"
            ]
            qbs.install: true
            qbs.installDir: grapheditor.INC_PATH + "/editor/graph"
            qbs.installPrefix: grapheditor.PREFIX
        }

        Group {
            name: "Install Dynamic Platform"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: grapheditor.LIB_PATH
            qbs.installPrefix: grapheditor.PREFIX
        }
    }
}
