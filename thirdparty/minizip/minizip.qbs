Project {
    id: minizip

    property stringList srcFiles: {
        var sources = [
            "ioapi.c",
            "miniunz.c",
            "mztools.c",
            "unzip.c",
            "zip.c"
        ];
        if(qbs.targetOS.contains("windows")) {
            sources.push("../minizip/iowin32.c")
        }

        return sources;
    }

    property stringList incPaths: [
        "./",
        "../zlib/src"
    ]

    StaticLibrary {
        name: "minizip"
        condition: minizip.desktop

        files: minizip.srcFiles
        Depends { name: "cpp" }

        cpp.includePaths: minizip.incPaths
        cpp.cxxLanguageVersion: minizip.languageVersion
        cpp.cxxStandardLibrary: minizip.standardLibrary
        cpp.minimumMacosVersion: minizip.osxVersion
    }
}
