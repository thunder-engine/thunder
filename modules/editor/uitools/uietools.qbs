import qbs

Project {
    id: uitools
    property stringList srcFiles: [
        "*.cpp",
        "editor/*.cpp",
        "editor/actions/*.cpp",
        "editor/tools/*.cpp",
        "converters/*.cpp",
        "*.qrc",
        "*.h",
        "editor/*.h",
        "editor/*.ui",
        "../../../thirdparty/pugixml/src/*.cpp"
    ]

    property stringList incPaths: [
        "editor",
        "../../../engine/includes",
        "../../../engine/includes/resources",
		"../../../engine/includes/components",
        "../../../editor/includes",
		"../../../modules/uikit/includes",
		"../../../modules/uikit/includes/resources",
		"../../../modules/uikit/includes/components",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/pugixml/src"
    ]

    DynamicLibrary {
        name: "uitools"
        condition: uitools.desktop
        files: uitools.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "uikit-editor" }
        Depends { name: "editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: uitools.incPaths
        cpp.cxxLanguageVersion: uitools.languageVersion
        cpp.cxxStandardLibrary: uitools.standardLibrary
        cpp.minimumMacosVersion: uitools.osxVersion

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
            qbs.installDir: uitools.PLUGINS_PATH
            qbs.installPrefix: uitools.PREFIX
        }
    }
}
