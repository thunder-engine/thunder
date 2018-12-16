import qbs
import qbs.FileInfo

Project {
    id: spirvcross
    property stringList srcFiles: [
        "src/*.cpp",
        "src/*.hpp"
    ]

    property stringList incPaths: [
        "src"
    ]

    StaticLibrary {
        name: "spirvcross"
        condition: spirvcross.desktop
        files: spirvcross.srcFiles
        Depends { name: "cpp" }
        bundle.isBundle: false

        cpp.defines: [ ]
        cpp.includePaths: spirvcross.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"
    }
}
