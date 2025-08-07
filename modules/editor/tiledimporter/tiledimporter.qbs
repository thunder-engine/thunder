import qbs

Project {
    id: tiledimporter
    property stringList srcFiles: [
        "*.cpp",
        "converter/*.cpp",
        "*.qrc",
        "*.h",
        "converter/**/*.h"
    ]

    property stringList incPaths: [
        "editor",
        "../../../engine/includes",
        "../../../engine/includes/resources",
        "../../../engine/includes/components",
        "../../../engine/includes/editor",
        "../../../thirdparty/pugixml/src",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core"
    ]

    DynamicLibrary {
        name: "tiledimporter"
        condition: tiledimporter.desktop
        files: tiledimporter.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: tiledimporter.incPaths
        cpp.cxxLanguageVersion: tiledimporter.languageVersion
        cpp.cxxStandardLibrary: tiledimporter.standardLibrary
        cpp.minimumMacosVersion: tiledimporter.osxVersion

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
            qbs.installDir: tiledimporter.PLUGINS_PATH
            qbs.installPrefix: tiledimporter.PREFIX
        }
    }
}
