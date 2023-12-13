import qbs

Project {
    id: iostools
    property stringList srcFiles: [
        "*.cpp",
        "converter/*.cpp",
        "*.qrc",
        "*.h",
        "converter/*.h",
    ]

    property stringList incPaths: [
        "../../../",
        "../../../engine/includes",
        "../../../engine/includes/resources",
        "../../../engine/includes/components",
        "../../../engine/includes/editor",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
    ]

    DynamicLibrary {
        name: "iostools"
        condition: iostools.desktop && qbs.targetOS.contains("darwin")
        files: iostools.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: iostools.incPaths
        cpp.cxxLanguageVersion: iostools.languageVersion
        cpp.cxxStandardLibrary: iostools.standardLibrary
        cpp.minimumMacosVersion: iostools.osxVersion

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Plugin"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: iostools.PLUGINS_PATH
            qbs.installPrefix: iostools.PREFIX
        }
    }
}
