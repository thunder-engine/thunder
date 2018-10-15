import qbs

Project {
    id: quazip
    property stringList srcFiles: [
        "src/*.c",
        "src/*.cpp",
        "src/*.h"
    ]

    property stringList incPaths: [
        "src",
        "../zlib/src"
    ]

    DynamicLibrary {
        name: "quazip-editor"
        condition: quazip.desktop
        files: quazip.srcFiles

        Depends { name: "cpp" }
        Depends { name: "zlib-editor" }
        Depends { name: "Qt"; submodules: ["core"]; }
        bundle.isBundle: false

        cpp.defines: ["QUAZIP_BUILD", "QUAZIP_BUILD", "NOMINMAX"]
        cpp.includePaths: quazip.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic zLib"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: quazip.BIN_PATH + "/" + quazip.bundle
            qbs.installPrefix: quazip.PREFIX
        }
    }
}
