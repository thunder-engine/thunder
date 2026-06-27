Project {
    id: minizip

    property stringList srcFiles: {
        var sources = [
            "compat/ioapi.c",
            "compat/unzip.c",
            "compat/zip.c",
            "mz_zip.c",
            "mz_os.c",
            "mz_strm.c",
            "mz_strm_buf.c",
            "mz_strm_mem.c",
            "mz_crypt.c"
        ];
        if(qbs.targetOS.contains("windows")) {
            sources.push("mz_os_win32.c")
            sources.push("mz_strm_os_win32.c")
        } else {
            sources.push("mz_os_posix.c")
            sources.push("mz_strm_os_posix.c")
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
