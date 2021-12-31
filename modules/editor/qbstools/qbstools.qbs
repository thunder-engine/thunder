import qbs

Project {
    id: qbstools
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
        "../../../engine/includes/editor",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
    ]

    DynamicLibrary {
        name: "qbstools"
        condition: qbstools.desktop
        files: qbstools.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui"]; }
        bundle.isBundle: false

        cpp.defines: ["NEXT_SHARED"]
        cpp.includePaths: qbstools.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Plugin"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: qbstools.PLUGINS_PATH
            qbs.installPrefix: qbstools.PREFIX
        }
    }
}
