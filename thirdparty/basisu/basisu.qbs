import qbs

Project {
    id: basisu
    property stringList srcFiles: [
        "encoder/**/*.cpp",
        "transcoder/**/*.cpp",
        "zstd/zstd.c"
    ]

    property stringList incPaths: [
        "encoder"
    ]

    StaticLibrary {
        name: "basisu"
        condition: basisu.desktop

        files: basisu.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: [  ]
        cpp.includePaths: basisu.incPaths
        cpp.cxxLanguageVersion: basisu.languageVersion
        cpp.cxxStandardLibrary: basisu.standardLibrary
    }
}
