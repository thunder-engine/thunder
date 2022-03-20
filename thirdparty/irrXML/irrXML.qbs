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
        cpp.cxxLanguageVersion: irrxml.languageVersion
        cpp.cxxStandardLibrary: irrxml.standardLibrary
        cpp.minimumMacosVersion: irrxml.osxVersion
    }
}
