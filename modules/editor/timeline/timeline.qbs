import qbs

Project {
    id: timeline
    property stringList srcFiles: [
        "*.cpp",
        "editor/**/*.cpp",
        "*.qrc",
        "*.h",
        "editor/**/*.h",
        "editor/*.ui",
    ]

    property stringList incPaths: [
        "editor",
        "../../../engine/includes",
        "../../../engine/includes/components",
        "../../../engine/includes/resources",
        "../../../engine/includes/editor",
        "../../../develop/managers/assetmanager",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/next/inc/anim",
        "../../../thirdparty/syntaxhighlighting/src"
    ]

    DynamicLibrary {
        name: "timeline"
        condition: timeline.desktop
        files: [
            "../../../worldeditor/res/WorldEditor.qrc",
        ].concat(timeline.srcFiles)
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }
        bundle.isBundle: false

        cpp.defines: {
            var result  = timeline.defines
            result.push("SHARED_DEFINE")
            return result
        }
        cpp.includePaths: timeline.incPaths
        cpp.cxxLanguageVersion: timeline.languageVersion
        cpp.cxxStandardLibrary: timeline.standardLibrary
        cpp.minimumMacosVersion: timeline.osxVersion

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Plugin"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: timeline.PLUGINS_PATH
            qbs.installPrefix: timeline.PREFIX
        }
    }
}
