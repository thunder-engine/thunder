import qbs

Project {
    id: vorbis
    property stringList srcFiles: [
        "src/mapping0.c",
        "src/lsp.c",
        "src/lpc.c",
        "src/lookup.c",
        "src/info.c",
        "src/floor1.c",
        "src/floor0.c",
        "src/envelope.c",
        "src/codebook.c",
        "src/block.c",
        "src/bitrate.c",
        "src/analysis.c",
        "src/window.c",
        "src/vorbisenc.c",
        "src/synthesis.c",
        "src/smallft.c",
        "src/sharedbook.c",
        "src/res0.c",
        "src/registry.c",
        "src/psy.c",
        "src/mdct.c"
    ]

    property stringList incPaths: [
        "src",
        "src/vorbis",
        "../libogg/src"
    ]

    DynamicLibrary {
        name: "vorbis-editor"
        condition: vorbis.desktop
        files: vorbis.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "ogg-editor" }
        bundle.isBundle: false

        cpp.defines: ["LIBVORBIS_LIBRARY"]
        cpp.includePaths: vorbis.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]
        cpp.sonamePrefix: "@executable_path"

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.linkerFlags: ["/DEF:" + path + "/src/vorbis.def"]
        }

        Group {
            name: "Install Dynamic Vorbis"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: vorbis.LIB_PATH + "/" + vorbis.bundle
            qbs.installPrefix: vorbis.PREFIX
        }
    }

    StaticLibrary {
        name: "vorbis"
        files: vorbis.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "ogg" }
        bundle.isBundle: false

        cpp.defines: [  ]
        cpp.includePaths: vorbis.incPaths

        Group {
            name: "Install Static Vorbis"
            condition: vorbis.desktop
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: vorbis.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: vorbis.PREFIX
        }
    }

    DynamicLibrary {
        name: "vorbisfile-editor"
        condition: vorbis.desktop
        files: [ "src/vorbisfile.c" ]
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "ogg-editor" }
        Depends { name: "vorbis-editor" }
        bundle.isBundle: false

        cpp.defines: ["LIBVORBIS_LIBRARY"]
        cpp.includePaths: vorbis.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]
        cpp.sonamePrefix: "@executable_path"

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.linkerFlags: ["/DEF:" + path + "/src/vorbisfile.def"]
        }

        Group {
            name: "Install Dynamic VorbisFile"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: vorbis.LIB_PATH + "/" + vorbis.bundle
            qbs.installPrefix: vorbis.PREFIX
        }
    }

    StaticLibrary {
        name: "vorbisfile"
        files: [ "src/vorbisfile.c" ]
        Depends { name: "cpp" }
        Depends { name: "ogg" }
        Depends { name: "vorbis" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: [  ]
        cpp.includePaths: "../libogg/src"

        Group {
            name: "Install Static VorbisFile"
            condition: vorbis.desktop
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: vorbis.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: vorbis.PREFIX
        }
    }
}
