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

    StaticLibrary {
        name: "quazip"
        condition: quazip.desktop
        files: quazip.srcFiles

        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "zlib-editor" }
        Depends { name: "Qt"; submodules: ["core5compat"]; }
        bundle.isBundle: false

        cpp.defines: ["QUAZIP_BUILD", "NOMINMAX"]
        cpp.includePaths: quazip.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]
    }
}
