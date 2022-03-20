import qbs

Project {
    id: particletools
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
        "../../../engine/includes/editor",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core"
    ]

    DynamicLibrary {
        name: "particletools"
        condition: particletools.desktop
        files: particletools.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets", "quickwidgets"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
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
