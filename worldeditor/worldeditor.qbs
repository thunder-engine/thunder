import qbs

Project {
    id: worldEditor
    property stringList srcFiles: [
        "src/**/*.qml",
        "src/**/*.ui",
        "src/**/*.cpp",
        "src/**/*.h",
        "../develop/**/*.cpp",
        "../develop/**/*.h",
        "res/icon.rc",
        "res/WorldEditor.qrc",
        "res/app-Info.plist"
    ]

    property stringList incPaths: [
        "src",
        "../",
        "../common",
        "../engine/includes",
        "../engine/includes/resources",
        "../develop/managers/projectmanager",
        "../develop/managers/assetmanager",
        "../develop/managers/pluginmanager",
        "../develop/managers/settingsmanager",
        "../develop/managers/undomanager",
        "../develop/models/include",
        "../modules/media/includes",
        "../thirdparty/assimp/include",
        "../thirdparty/next/inc",
        "../thirdparty/next/inc/math",
        "../thirdparty/next/inc/core",
        "../thirdparty/next/inc/anim",
        "../thirdparty/physfs/inc",
        "../thirdparty/glfw/inc",
        "../thirdparty/zlib/src",
        "../thirdparty/libogg/src",
        "../thirdparty/libvorbis/src",
        "../thirdparty/glsl",
        "../thirdparty/spirvcross/src",
        "../thirdparty/syntaxhighlighting/src",
        "../thirdparty/ofbx/src"
    ]

    QtGuiApplication {
        name: worldEditor.EDITOR_NAME
        condition: worldEditor.desktop
        files: worldEditor.srcFiles

        Depends { name: "cpp" }
        Depends { name: "assimp" }
        Depends { name: "bundle" }
        Depends { name: "zlib-editor" }
        Depends { name: "next-editor" }
        Depends { name: "vorbis-editor" }
        Depends { name: "ogg-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "glsl" }
        Depends { name: "spirvcross" }
        Depends { name: "syntaxhighlighting" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets", "multimedia", "quickwidgets"]; }
        property bool isBundle: qbs.targetOS.contains("darwin") && bundle.isBundle
        bundle.infoPlist: ({
            "NSHumanReadableCopyright": "(C) 2007-" + worldEditor.COPYRIGHT_YEAR + " by " + worldEditor.COPYRIGHT_AUTHOR
        })
        bundle.identifierPrefix: "com.thunderengine"

        consoleApplication: false

        cpp.defines: {
            var result  = worldEditor.defines
            result.push("NEXT_SHARED")
            return result
        }

        cpp.includePaths: worldEditor.incPaths
        property string prefix: qbs.targetOS.contains("windows") ? "lib" : ""
        cpp.cxxLanguageVersion: "c++14"

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.dynamicLibraries: outer.concat([
                "opengl32",
                "glu32"
            ])
        }

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@rpath"
            cpp.rpaths: "@executable_path/../Frameworks/"
            cpp.weakFrameworks: ["OpenGL"]
        }

        Group {
            name: "Install " + worldEditor.EDITOR_NAME
            qbs.install: true
            qbs.installDir: worldEditor.BIN_PATH
            qbs.installPrefix: worldEditor.PREFIX

            fileTagsFilter: isBundle ? ["bundle.content"] : ["application"]
            qbs.installSourceBase: product.buildDirectory
        }

        Group {
            name: "Icon"
            qbs.install: qbs.targetOS.contains("darwin")
            files: [
                "res/icons/thunder.icns"
            ]
            qbs.installDir: worldEditor.BIN_PATH + "/" + worldEditor.bundle + "../Resources"
            qbs.installPrefix: worldEditor.PREFIX
        }
    }
}
