import qbs

Project {
    id: webtools
    property stringList srcFiles: [
        "*.cpp",
        "converter/*.cpp",
        "*.h",
        "converter/**/*.h",
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
        name: "webtools"
        condition: webtools.desktop
        files: [
            "webtools.qrc",
        ].concat(webtools.srcFiles)
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: webtools.incPaths
        cpp.cxxLanguageVersion: webtools.languageVersion
        cpp.cxxStandardLibrary: webtools.standardLibrary
        cpp.minimumMacosVersion: webtools.osxVersion

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
            qbs.installDir: webtools.PLUGINS_PATH
            qbs.installPrefix: webtools.PREFIX
        }
    }
}
