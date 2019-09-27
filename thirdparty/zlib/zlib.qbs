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
        condition: zlib.desktop
        files: zlib.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: ["ZLIB_DLL", "ZLIB_LIBRARY"]
        cpp.includePaths: zlib.incPaths
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
            qbs.installDir: zlib.LIB_PATH + "/" + zlib.bundle
            qbs.installPrefix: zlib.PREFIX
        }
    }

    StaticLibrary {
        name: "zlib"
        files: zlib.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: [  ]
        cpp.includePaths: zlib.incPaths

        Group {
            name: "Install Static zLib"
            condition: zlib.desktop
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: zlib.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: zlib.PREFIX
        }
    }
}
