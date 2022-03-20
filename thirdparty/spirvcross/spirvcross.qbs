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
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: [ ]
        cpp.includePaths: spirvcross.incPaths
        cpp.cxxLanguageVersion: spirvcross.languageVersion
        cpp.cxxStandardLibrary: spirvcross.standardLibrary
        cpp.minimumMacosVersion: spirvcross.osxVersion
    }
}
