import qbs

Project {
    id: texteditor
    property stringList srcFiles: [
        "*.cpp",
        "editor/*.cpp",
        "*.qrc",
        "*.h",
        "editor/*.h",
        "editor/*.ui",
    ]

    property stringList incPaths: [
        "editor",
        "../../../engine/includes",
        "../../../engine/includes/resources",
        "../../../engine/includes/editor",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/syntaxhighlighting/src"
    ]

    DynamicLibrary {
        name: "texteditor"
        condition: texteditor.desktop
        files: texteditor.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "syntaxhighlighting" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: texteditor.incPaths
        cpp.cxxLanguageVersion: texteditor.languageVersion
        cpp.cxxStandardLibrary: texteditor.standardLibrary
        cpp.minimumMacosVersion: texteditor.osxVersion

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Plugin"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: texteditor.PLUGINS_PATH
            qbs.installPrefix: texteditor.PREFIX
        }
    }
}
