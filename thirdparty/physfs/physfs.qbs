import qbs

Project {
    id: physfs
    property stringList srcFiles: {
        var result = [
            "src/*.c",
            "src/archivers/*.c"
        ];
        if(qbs.targetOS.contains("windows")) {
            result.push("src/platform/win32.c")
        } else {
            result.push("src/platform/unix.c"),
            result.push("src/platform/posix.c")
        }
        return result;
    }

    property stringList incPaths: [
        "src",
        "../zlib/src"
    ]

    DynamicLibrary {
        name: "physfs-editor"
        condition: physfs.desktop
        files: physfs.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "zlib-editor" }
        bundle.isBundle: false

        cpp.defines: ["PHYSFS_SUPPORTS_ZIP", "PHYSFS_NO_CDROM_SUPPORT"]
        cpp.includePaths: physfs.incPaths
        cpp.libraryPaths: [ ]

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.dynamicLibraries: [ "Advapi32" ]
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.defines: outer.concat(["PHYSFS_DARWIN"])
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic physfs"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: physfs.LIB_PATH
            qbs.installPrefix: physfs.PREFIX
        }
    }

    StaticLibrary {
        name: "physfs"
        files: physfs.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: ["PHYSFS_SUPPORTS_ZIP", "PHYSFS_NO_CDROM_SUPPORT"]
        cpp.includePaths: physfs.incPaths

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.defines: outer.concat(["PHYSFS_DARWIN"])
        }

        Group {
            name: "Install Static physfs"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: physfs.STATIC_PATH
            qbs.installPrefix: physfs.PREFIX
        }
    }
}
