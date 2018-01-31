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
        } else if(qbs.targetOS.contains("osx")) {
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
        Depends { name: "zlib-editor" }
        bundle.isBundle: false

        cpp.defines: ["PHYSFS_SUPPORTS_ZIP", "PHYSFS_NO_CDROM_SUPPORT"]
        cpp.includePaths: physfs.incPaths
        cpp.libraryPaths: [ ]
        cpp.sonamePrefix: "@executable_path"

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.dynamicLibraries: [ "Advapi32" ]
        }

        Properties {
            condition: qbs.targetOS.contains("osx")
            cpp.defines: outer.concat(["PHYSFS_DARWIN"])
        }

        Group {
            name: "Install Dynamic physfs"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: physfs.BIN_PATH + "/" + physfs.bundle
            qbs.installPrefix: physfs.PREFIX
        }
    }

    StaticLibrary {
        name: "physfs"
        files: physfs.srcFiles
        Depends { name: "cpp" }
        bundle.isBundle: false

        cpp.defines: ["PHYSFS_SUPPORTS_ZIP", "PHYSFS_NO_CDROM_SUPPORT"]
        cpp.includePaths: physfs.incPaths

        Properties {
            condition: qbs.targetOS.contains("osx")
            cpp.defines: outer.concat(["PHYSFS_DARWIN"])
        }

        Group {
            name: "Install Static physfs"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: physfs.LIB_PATH
            qbs.installPrefix: physfs.PREFIX
        }
    }
}
