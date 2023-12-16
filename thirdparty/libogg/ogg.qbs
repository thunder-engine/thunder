import qbs

Project {
    id: ogg
    property stringList srcFiles: [
        "src/*.c"
    ]

    property stringList incPaths: [
        "src",
        "src/ogg"
    ]

    DynamicLibrary {
        name: "ogg-editor"
        condition: ogg.desktop
        files: ogg.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: ["LIBOGG_LIBRARY"]
        cpp.includePaths: ogg.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]

        Properties {
            condition: qbs.targetOS.contains("windows") && !qbs.toolchain.contains("gcc")
            cpp.linkerFlags: ["/DEF:" + path + "/src/ogg.def"]
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic Ogg"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: ogg.LIB_PATH
            qbs.installPrefix: ogg.PREFIX
        }
    }

    StaticLibrary {
        name: "ogg"
        files: ogg.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: [  ]
        cpp.includePaths: ogg.incPaths

        Group {
            name: "Install Static Ogg"
            condition: ogg.desktop
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: ogg.STATIC_PATH
            qbs.installPrefix: ogg.PREFIX
        }
    }
}
