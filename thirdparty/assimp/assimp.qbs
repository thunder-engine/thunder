import qbs

Project {
    id: assimp
    property stringList srcFiles: {
        var sources = [ 
        "code/**/*.cpp",
        "code/**/*.h",
        "../minizip/ioapi.c",
        "../minizip/miniunz.c",
        "../minizip/mztools.c",
        "../minizip/unzip.c",
        "../minizip/zip.c",
        "../unzip/*.c",
        "../unzip/*.h"
        ];
        if(qbs.targetOS.contains("windows")) {
            sources.push("../minizip/iowin32.c")
        }

        return sources;
    }

    property stringList incPaths: [
        "./",
        "../",
        "include",
        "code",
        "../minizip",
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
