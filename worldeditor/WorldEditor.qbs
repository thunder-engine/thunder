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
        "res/WorldEditor.qrc"
    ]

    property stringList incPaths: [
        "src",
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
            "LAUNCHER_VERSION=\"" + LAUNCHER_VERSION + "\"",
            "YEAR=" + YEAR
        ];

        return result;
    }

    QtGuiApplication {
        name: worldEditor.EDITOR_NAME
        files: worldEditor.srcFiles
        Depends { name: "cpp" }
        Depends { name: "zlib-editor" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "rendergl-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "opengl"]; }

        consoleApplication: false

        cpp.defines: worldEditor.defines
        cpp.includePaths: worldEditor.incPaths
        cpp.libraryPaths: [
            "../thirdparty/fbx/lib"
        ]

        cpp.dynamicLibraries: [
            "fbxsdk-2012.1",
            "opengl32",
            "glu32"
        ]

        Group {
            name: "Install " + worldEditor.EDITOR_NAME
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: worldEditor.BIN_PATH
            qbs.installPrefix: worldEditor.PREFIX
        }
    }
}
