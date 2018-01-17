import qbs

Project {
    id: zlib
    property stringList srcFiles: [
        "src/*.c"
    ]

    property stringList incPaths: [
        "src"
    ]

    DynamicLibrary {
        name: "zlib-editor"
        files: zlib.srcFiles
        Depends { name: "cpp" }

        cpp.defines: ["ZLIB_DLL", "ZLIB_LIBRARY"]
        cpp.includePaths: zlib.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]

        Group {
            name: "Install Dynamic zLib"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: zlib.BIN_PATH
            qbs.installPrefix: zlib.PREFIX
        }
    }

    StaticLibrary {
        name: "zlib"
        files: zlib.srcFiles
        Depends { name: "cpp" }

        cpp.defines: [  ]
        cpp.includePaths: zlib.incPaths

        Group {
            name: "Install Static zLib"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: zlib.LIB_PATH
            qbs.installPrefix: zlib.PREFIX
        }
    }
}
