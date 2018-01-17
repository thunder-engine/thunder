import qbs

Project {
    id: next
    property stringList srcFiles: [
        "src/*.cpp",
        "src/core/*.cpp",
        "src/math/*.cpp"
    ]

    property stringList incPaths: [
        "inc",
        "inc/core",
        "inc/math"
    ]

    DynamicLibrary {
        name: "next-editor"
        files: next.srcFiles
        Depends { name: "cpp" }

        cpp.defines: ["BUILD_SHARED", "NEXT_LIBRARY"]
        cpp.includePaths: next.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]

        Group {
            name: "Install Dynamic Platform"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: next.BIN_PATH
            qbs.installPrefix: next.PREFIX
        }
    }

    StaticLibrary {
        name: "next"
        files: next.srcFiles
        Depends { name: "cpp" }

        cpp.defines: ["NEXT_LIBRARY"]
        cpp.includePaths: next.incPaths

        Group {
            name: "Install Static Platform"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: next.LIB_PATH
            qbs.installPrefix: next.PREFIX
        }
    }
}
