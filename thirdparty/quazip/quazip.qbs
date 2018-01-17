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
        files: quazip.srcFiles
        Depends { name: "cpp" }
        Depends { name: "zlib-editor" }
        Depends { name: "Qt"; submodules: ["core"]; }

        cpp.defines: ["QUAZIP_BUILD", "QUAZIP_BUILD", "NOMINMAX"]
        cpp.includePaths: quazip.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]

        Group {
            name: "Install Dynamic zLib"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: quazip.BIN_PATH
            qbs.installPrefix: quazip.PREFIX
        }
    }
}
