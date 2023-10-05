import qbs

Project {
    id: motiontools
    property stringList srcFiles: [
        "*.cpp",
        "converter/*.cpp",
        "editor/*.cpp",
        "editor/*.ui",
        "*.qrc",
        "*.h",
        "converter/**/*.h",
        "editor/**/*.h"
    ]

    property stringList incPaths: [
        "editor",
        "../../../engine/includes",
        "../../../engine/includes/resources",
        "../../../engine/includes/editor",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/next/inc/anim",
        "../../../modules/editor/grapheditor"
    ]

    DynamicLibrary {
        name: "motiontools"
        condition: motiontools.desktop
        files: motiontools.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "graph-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui","widgets", "xml"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: motiontools.incPaths
        cpp.cxxLanguageVersion: motiontools.languageVersion
        cpp.cxxStandardLibrary: motiontools.standardLibrary
        cpp.minimumMacosVersion: motiontools.osxVersion

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
            qbs.installDir: motiontools.PLUGINS_PATH
            qbs.installPrefix: motiontools.PREFIX
        }
    }
}
