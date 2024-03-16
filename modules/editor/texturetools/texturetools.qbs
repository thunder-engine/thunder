import qbs

Project {
    id: texturetools
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
        "editor",
        "../../../engine/includes",
        "../../../engine/includes/components",
        "../../../engine/includes/resources",
        "../../../engine/includes/editor",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core"
    ]

    DynamicLibrary {
        name: "texturetools"
        condition: texturetools.desktop
        files: texturetools.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: texturetools.incPaths
        cpp.cxxLanguageVersion: texturetools.languageVersion
        cpp.cxxStandardLibrary: texturetools.standardLibrary
        cpp.minimumMacosVersion: texturetools.osxVersion

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
            qbs.installDir: texturetools.PLUGINS_PATH
            qbs.installPrefix: texturetools.PREFIX
        }
    }
}
