import qbs

Project {
    id: editor
    property stringList srcFiles: [
        "src/**/*.cpp"
    ]

    property stringList incPaths: [
        "../",
        "includes",
        "includes/editor",
        "../engine/includes",
        "../engine/includes/components",
        "../engine/includes/resources",
        "../thirdparty/next/inc",
        "../thirdparty/next/inc/math",
        "../thirdparty/next/inc/core",
        "../thirdparty/next/inc/anim",
        "../thirdparty/next/inc/os",
        "../thirdparty/assimp/include",
        "../thirdparty/pugixml/src"
    ]

    DynamicLibrary {
        name: "editor"
        condition: editor.desktop
        files: editor.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "assimp" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "zlib-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets", "svg"]; }
        bundle.isBundle: false

        cpp.defines: {
            var result = editor.defines
            result.push("SHARED_DEFINE")
            result.push("EDITOR_LIBRARY")
            return result
        }
        cpp.includePaths: editor.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]
        cpp.cxxLanguageVersion: editor.languageVersion
        cpp.cxxStandardLibrary: editor.standardLibrary
        cpp.minimumMacosVersion: editor.osxVersion

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["IOKit"]
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic Editor"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: editor.LIB_PATH
            qbs.installPrefix: editor.PREFIX
        }

        Group {
            name: "Editor includes"
            prefix: "includes/editor"
            files: [
                "**/*.h",
            ]
            qbs.install: true
            qbs.installDir: editor.INC_PATH + "/editor"
            qbs.installPrefix: editor.PREFIX
        }

        Group {
            name: "Viewport Editor includes"
            prefix: "includes/viewport"
            files: [
                "**/*.h"
            ]
            qbs.install: true
            qbs.installDir: editor.INC_PATH + "/viewport"
            qbs.installPrefix: editor.PREFIX
        }
    }
}
