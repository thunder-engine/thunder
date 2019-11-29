import qbs

Project {
    id: builder
    property stringList srcFiles: [
        "**/*.cpp",
        "**/*.h",
        "../develop/**/*.cpp",
        "../develop/**/*.h"
    ]

    property stringList incPaths: [
        "src",
        "../",
        "../common",
        "../engine/includes",
        "../engine/includes/resources",
        "../develop/managers/codemanager/include",
        "../develop/managers/projectmanager/include",
        "../develop/managers/assetmanager/include",
        "../develop/managers/pluginmanager/include",
        "../develop/managers/settingsmanager",
        "../develop/managers/undomanager",
        "../develop/models/include",
        "../modules/renders/rendergl/includes",
        "../thirdparty/next/inc",
        "../thirdparty/next/inc/math",
        "../thirdparty/next/inc/core",
        "../thirdparty/next/inc/anim",
        "../thirdparty/physfs/inc",
        "../thirdparty/glfw/inc",
        "../thirdparty/fbx/inc",
        "../thirdparty/zlib/src",
        "../thirdparty/quazip/src",
        "../thirdparty/glsl",
        "../thirdparty/spirvcross/src"
    ]

    QtApplication {
        name: builder.BUILDER_NAME
        condition: builder.desktop
        files: builder.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "zlib-editor" }
        Depends { name: "quazip-editor" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "glsl" }
        Depends { name: "spirvcross" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }

        bundle.isBundle: false

        cpp.defines: {
            var result = builder.defines
            result.push("NEXT_SHARED")
            result.push("BUILDER")
            return result
        }
        cpp.includePaths: builder.incPaths
        cpp.libraryPaths: {
            var arch = "/x86"
            if(qbs.architecture !== "x86") {
                arch = "/x64"
            }
            return "../thirdparty/fbx/lib/" + qbs.targetOS[0] + arch
        }

        property string prefix: qbs.targetOS.contains("windows") ? "lib" : ""
        cpp.dynamicLibraries: [
            prefix + "fbxsdk"
        ]
        cpp.cxxLanguageVersion: "c++14"

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@rpath"
            cpp.rpaths: "@executable_path/../Frameworks/"
        }

        Group {
            name: "Install " + builder.BUILDER_NAME
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: builder.BIN_PATH + "/" + builder.bundle
            qbs.installPrefix: builder.PREFIX
        }
    }
}
