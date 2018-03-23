import qbs

Project {
    id: worldEditor
    property stringList srcFiles: [
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
        "../develop/managers/codemanager/include",
        "../develop/managers/projectmanager/include",
        "../develop/managers/assetmanager/include",
        "../develop/models/include",
        "../modules/renders/rendergl/includes",
        "../thirdparty/next/inc",
        "../thirdparty/next/inc/math",
        "../thirdparty/next/inc/core",
        "../thirdparty/physfs/inc",
        "../thirdparty/glfw/inc",
        "../thirdparty/fbx/inc",
        "../thirdparty/zlib/src"
    ]

    property stringList defines: {
        var result  = [
            "COMPANY_NAME=\"" + COMPANY_NAME + "\"",
            "EDITOR_NAME=\"" + EDITOR_NAME + "\"",
            "BUILDER_NAME=\"" + BUILDER_NAME + "\"",
            "SDK_VERSION=\"" + SDK_VERSION + "\"",
            "COPYRIGHT_YEAR=" + COPYRIGHT_YEAR
        ];
        return result;
    }

    QtGuiApplication {
        name: worldEditor.EDITOR_NAME
        condition: worldEditor.desktop
        files: worldEditor.srcFiles

        Depends { name: "cpp" }
        Depends { name: "zlib-editor" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "rendergl-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }

        property bool isBundle: qbs.targetOS.contains("darwin") && bundle.isBundle
        bundle.infoPlist: ({
            "NSHumanReadableCopyright": "(C) 2007-" + worldEditor.COPYRIGHT_YEAR + " by " + worldEditor.COPYRIGHT_AUTHOR
        })
        bundle.identifierPrefix: "com.thunderengine"

        consoleApplication: false

        cpp.defines: worldEditor.defines
        cpp.includePaths: worldEditor.incPaths
        cpp.libraryPaths: [
            "../thirdparty/fbx/lib"
        ]
        property string prefix: qbs.targetOS.contains("windows") ? "lib" : ""
        cpp.dynamicLibraries: [
            prefix + "fbxsdk"
        ]
        cpp.cxxLanguageVersion: "c++14"

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.dynamicLibraries: outer.concat([
                "opengl32",
                "glu32"
            ])
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
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
