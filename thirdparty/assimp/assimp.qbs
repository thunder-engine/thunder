import qbs

Project {
    id: assimp
    property stringList srcFiles: [
        "code/**/*.cpp",
        "code/**/*.h",
        "../unzip/*.c",
        "../unzip/*.h"
    ]

    property stringList incPaths: [
        "./",
        "../",
        "include",
        "code",
        "../pugixml/src",
        "../poly2tri",
        "../zlib/src",
        "../rapidjson/include",
        "../utf8cpp/source",
        "../unzip/"
    ]

    StaticLibrary {
        name: "assimp"
        condition: assimp.desktop

        cpp.defines: [
            "ASSIMP_USE_HUNTER",
            "ASSIMP_BUILD_NO_OWN_ZLIB",
            "ASSIMP_BUILD_NO_EXPORT",
            "ASSIMP_BUILD_NO_IFC_IMPORTER",
            "ASSIMP_BUILD_NO_OPENGEX_IMPORTER",
            "ASSIMP_BUILD_NO_C4D_IMPORTER",
            "ASSIMP_BUILD_NO_STEP_IMPORTER",
            "ASSIMP_BUILD_NO_OGRE_IMPORTER",
            "ASSIMP_BUILD_NO_IRR_IMPORTER",
            "ASSIMP_BUILD_NO_SIB_IMPORTER",
            "ASSIMP_BUILD_NO_COLLADA_IMPORTER",
            "ASSIMP_BUILD_NO_MMD_IMPORTER"
        ]

        files: assimp.srcFiles
        Depends { name: "cpp" }
        Depends { name: "pugixml" }

        cpp.includePaths: assimp.incPaths
        cpp.cxxLanguageVersion: assimp.languageVersion
        cpp.cxxStandardLibrary: assimp.standardLibrary
        cpp.minimumMacosVersion: assimp.osxVersion
    }
}
