import qbs

Project {
    id: irrxml
    property stringList srcFiles: [
        "src/**/*.cpp",
        "src/**/*.h"
    ]

    property stringList incPaths: [
        "src"
    ]

    StaticLibrary {
        name: "irrxml"
        files: irrxml.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: irrxml.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.cxxStandardLibrary: "libc++"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"
    }
}
