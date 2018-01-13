import qbs

Project {
    id: rendergl
    property stringList srcFiles: [
        "src/components/acameragl.cpp",
        "src/components/aeffectgl.cpp",
        "src/components/alightsourcegl.cpp",
        "src/resources/amaterialgl.cpp",
        "src/components/aspritegl.cpp",
        "src/components/astaticmeshgl.cpp",
        "src/components/atextgl.cpp",
        "src/postprocess/abloomgl.cpp",
        "src/resources/ameshgl.cpp",
        "src/resources/atexturegl.cpp",
        "src/adecal.cpp",
        "src/adeferredshading.cpp",
        "src/apipeline.cpp",
        "src/postprocess/aantialiasinggl.cpp",
        "src/postprocess/aambientocclusiongl.cpp",
        "src/filters/ablurgl.cpp",
        "src/arenderglsystem.cpp",
        "src/arendergl.cpp",

        "includes/components/acameragl.h",
        "includes/components/aeffectgl.h",
        "includes/components/alightsourcegl.h",
        "includes/components/aspritegl.h",
        "includes/components/astaticmeshgl.h",
        "includes/components/atextgl.h",

        "includes/resources/ameshgl.h",
        "includes/resources/atexturegl.h",
        "includes/resources/amaterialgl.h",

        "includes/postprocess/abloomgl.h",

        "includes/filters/ablurgl.h",

        "includes/postprocess/aantialiasinggl.h",
        "includes/postprocess/aambientocclusiongl.h",
        "includes/postprocess/apostprocessor.h",

        "includes/adecal.h",
        "includes/adeferredshading.h",
        "includes/agl.h",
        "includes/apipeline.h",
        "includes/arenderglsystem.h",
        "includes/arendergl.h"
    ]

    property stringList incPaths: [
        "includes",
        "../../../common",
        "../../../engine/includes",
        "../../../../external/next/inc",
        "../../../../external/next/inc/math",
        "../../../../external/next/inc/core",
        "../../../../external/nvidia/inc"
    ]

    DynamicLibrary {
        name: "rendergl-editor"
        files: rendergl.srcFiles
        Depends { name: "cpp" }
        Depends {
            name: "engine-editor"
        }

        cpp.defines: ["BUILD_SHARED", "NEXT_LIBRARY"]
        cpp.includePaths: rendergl.incPaths
        cpp.libraryPaths: [
            "../../bin",
            "../../../../external-target/libs/next/shared/windows_x32/release",
            "../../../../external/nvidia/lib"
        ]
        cpp.dynamicLibraries: [
            "next",
            "glew32",
            "opengl32"
        ]

        Group {
            name: "Install Dynamic Engine"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: "bin"
            qbs.installPrefix: "../../"
        }
    }

    StaticLibrary {
        name: "rendergl"
        files: rendergl.srcFiles
        Depends { name: "cpp" }

        cpp.includePaths: rendergl.incPaths

        Group {
            name: "Install Static Engine"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: "lib"
            qbs.installPrefix: "../../"
        }
    }
}
