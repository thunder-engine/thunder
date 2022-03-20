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
        "../develop/managers/assetmanager",
        "../develop/models/include",
        "../thirdparty/assimp/include",
        "../thirdparty/next/inc",
        "../thirdparty/next/inc/math",
        "../thirdparty/next/inc/core",
        "../thirdparty/next/inc/anim",
        "../thirdparty/zlib/src",
        "../thirdparty/quazip/src"
    ]

    QtApplication {
        name: builder.BUILDER_NAME
        condition: builder.desktop
        files: builder.srcFiles
        Depends { name: "cpp" }
        Depends { name: "assimp" }
        Depends { name: "bundle" }
        Depends { name: "zlib-editor" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }

        bundle.isBundle: false

        cpp.defines: {
            var result = builder.defines
            result.push("SHARED_DEFINE")
            result.push("BUILDER")
            result.push("QUAZIP_STATIC")
            return result
        }

        cpp.includePaths: builder.incPaths

        property string prefix: qbs.targetOS.contains("windows") ? "lib" : ""
        cpp.dynamicLibraries: [ ]
        cpp.cxxLanguageVersion: builder.languageVersion

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
