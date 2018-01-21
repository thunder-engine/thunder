import qbs

Project {
    id: physfs
    property stringList srcFiles: [
        "src/*.c",
        "src/archivers/*.c",

        "src/platform/win32.c"
    ]

    property stringList incPaths: [
        "src",
        "../zlib/src"
    ]

    DynamicLibrary {
        name: "physfs-editor"
        condition: physfs.desktop
        files: physfs.srcFiles
        Depends { name: "cpp" }
        Depends { name: "zlib-editor" }

        cpp.defines: ["PHYSFS_SUPPORTS_ZIP"]
        cpp.includePaths: physfs.incPaths
        cpp.libraryPaths: [ ]

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.dynamicLibraries: [ "Advapi32" ]
        }

        Group {
            name: "Install Dynamic physfs"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: physfs.BIN_PATH
            qbs.installPrefix: physfs.PREFIX
        }
    }

    StaticLibrary {
        name: "physfs"
        files: physfs.srcFiles
        Depends { name: "cpp" }

        cpp.defines: ["PHYSFS_SUPPORTS_ZIP"]
        cpp.includePaths: physfs.incPaths

        Group {
            name: "Install Static physfs"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: physfs.LIB_PATH
            qbs.installPrefix: physfs.PREFIX
        }
    }
}
