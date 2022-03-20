import qbs

Project {
    id: assimp
    property stringList srcFiles: [
        "code/**/*.cpp",
        "code/**/*.h"
    ]

    property stringList incPaths: [
        "./",
        "include",
        "code",
        "../irrXML/src",
        "../poly2tri",
        "../zlib/src",
        "../utf8cpp",
        "../quazip/src"
    ]

    StaticLibrary {
        name: "assimp"
        condition: assimp.desktop

        cpp.defines: [
            "ASSIMP_BUILD_NO_OWN_ZLIB",
            "ASSIMP_USE_HUNTER",
            "ASSIMP_BUILD_NO_GLTF_IMPORTER",
            "ASSIMP_BUILD_NO_IFC_IMPORTER",
            "ASSIMP_BUILD_NO_OPENGEX_IMPORTER",
            "ASSIMP_BUILD_NO_C4D_IMPORTER",
            "ASSIMP_BUILD_NO_3MF_EXPORTER",
            "ASSIMP_BUILD_NO_STEP_IMPORTER"]

        files: assimp.srcFiles
        Depends { name: "cpp" }
        Depends { name: "irrxml" }
        Depends { name: "quazip" }

        cpp.includePaths: assimp.incPaths
        cpp.cxxLanguageVersion: assimp.languageVersion
        cpp.cxxStandardLibrary: assimp.standardLibrary
        cpp.minimumMacosVersion: assimp.osxVersion
    }
}
