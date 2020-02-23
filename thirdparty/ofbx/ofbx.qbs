import qbs

Project {
    id: ofbx
    property stringList srcFiles: [
        "src/miniz.c",
        "src/ofbx.cpp"
    ]

    property stringList incPaths: [
        "src"
    ]

    StaticLibrary {
        name: "ofbx"
        condition: ofbx.desktop
        files: ofbx.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: [ ]
        cpp.cxxLanguageVersion: "c++14"
        cpp.includePaths: ofbx.incPaths
    }
}
