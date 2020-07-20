import qbs

Project {
    id: poly2tri
    property stringList srcFiles: [
        "poly2tri/**/*.cpp",
        "poly2tri/**/*.h"
    ]

    property stringList incPaths: [
    ]

    StaticLibrary {
        name: "poly2tri"
        files: poly2tri.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: poly2tri.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.cxxStandardLibrary: "libc++"
        cpp.minimumMacosVersion: "10.12"
        cpp.minimumIosVersion: "10.0"
        cpp.minimumTvosVersion: "10.0"
    }
}
