import qbs

Project {
    id: spineimporter
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
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/next/inc/anim"
    ]

    DynamicLibrary {
        name: "spineimporter"
        condition: spineimporter.desktop
        files: spineimporter.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: spineimporter.incPaths
        cpp.cxxLanguageVersion: spineimporter.languageVersion
        cpp.cxxStandardLibrary: spineimporter.standardLibrary
        cpp.minimumMacosVersion: spineimporter.osxVersion

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path/plugins"
        }

        Group {
            name: "Install Plugin"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: spineimporter.PLUGINS_PATH
            qbs.installPrefix: spineimporter.PREFIX
        }
    }
}
