import qbs

Project {
    id: uikitFormEditor

    property stringList srcFiles: [
        "../src/converters/*.cpp",
        "../src/editor/**/*.cpp",
        "../src/editor/**/*.h",
        "../includes/converters/*.h",
        "../includes/editor/**/*.h",
        "../src/editor/*.ui",
        "../src/editor/*.qrc",
        "../../../thirdparty/pugixml/src/*.cpp",
        "formeditor.cpp"
    ]

    property stringList incPaths: [
        "../includes",
        "../includes/components",
        "../includes/resources",
        "../includes/editor",
        "../includes/editor/tools",
        "../../../engine/includes",
        "../../../engine/includes/components",
        "../../../engine/includes/resources",
        "../../../editor/includes",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/pugixml/src",
        "../../../thirdparty/cssparser/include"
    ]

    DynamicLibrary {
        name: "uikit-formeditor"
        condition: uikitFormEditor.desktop
        files: uikitFormEditor.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "uikit-editor" }
        Depends { name: "editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }
        bundle.isBundle: false

        cpp.defines: {
            var result = uikitFormEditor.defines
            result.push("SHARED_DEFINE")
            return result
        }
        cpp.includePaths: uikitFormEditor.incPaths
        cpp.cxxLanguageVersion: uikitFormEditor.languageVersion
        cpp.cxxStandardLibrary: uikitFormEditor.standardLibrary
        cpp.minimumMacosVersion: uikitFormEditor.osxVersion

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install UiKit Form Editor"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: uikitFormEditor.LIB_PATH
            qbs.installPrefix: uikitFormEditor.PREFIX
        }
    }
}
