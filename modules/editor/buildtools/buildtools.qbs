import qbs

Project {
    id: buildtools
    property stringList srcFiles: [
        "*.cpp",
        "converter/*.cpp",
        "*.qrc",
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
        "../../../thirdparty/zlib/src",
        "../../../thirdparty/minizip"
    ]

    DynamicLibrary {
        name: "buildtools"
        condition: buildtools.desktop
        files: buildtools.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "minizip" }
        Depends { name: "zlib-editor" }
        Depends { name: "Qt"; submodules: ["gui"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: buildtools.incPaths
        cpp.cxxLanguageVersion: buildtools.languageVersion
        cpp.cxxStandardLibrary: buildtools.standardLibrary
        cpp.minimumMacosVersion: buildtools.osxVersion

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
            qbs.installDir: buildtools.PLUGINS_PATH
            qbs.installPrefix: buildtools.PREFIX
        }
    }
}
