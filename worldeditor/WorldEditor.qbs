import qbs

Project {
    id: worldBuilder
    property stringList srcFiles: [
        "src/components/actor.cpp",
        "src/components/camera.cpp",
        "src/components/chunk.cpp",
        "src/components/component.cpp",
        "src/components/effect.cpp",
        "src/components/lightsource.cpp",
        "src/components/scene.cpp",
        "src/components/sprite.cpp",
        "src/components/staticmesh.cpp",
        "src/analytics/profiler.cpp",
        "src/adapters/desktopadaptor.cpp",
        "src/resources/font.cpp",
        "src/engine.cpp",
        "src/input.cpp",
        "src/resources/mesh.cpp",
        "src/resources/texture.cpp",
        "src/resources/animation.cpp",
        "src/timer.cpp",
        "src/resources/material.cpp",
        "src/resources/text.cpp",
        "src/components/screentext.cpp",
        "src/controller.cpp",
        "src/file.cpp",

        "includes/analytics/profiler.h",

        "includes/adapters/iplatformadaptor.h",
        "includes/adapters/desktopadaptor.h",

        "includes/components/actor.h",
        "includes/components/camera.h",
        "includes/components/chunk.h",
        "includes/components/component.h",
        "includes/components/effect.h",
        "includes/components/lightsource.h",
        "includes/components/scene.h",
        "includes/components/sprite.h",
        "includes/components/staticmesh.h",
        "includes/components/screentext.h",

        "includes/resources/animation.h",
        "includes/resources/font.h",
        "includes/resources/mesh.h",
        "includes/resources/texture.h",
        "includes/resources/material.h",
        "includes/resources/text.h",

        "includes/engine.h",
        "includes/input.h",
        "includes/log.h",
        "includes/system.h",
        "includes/controller.h",
        "includes/module.h",
        "includes/file.h",
        "includes/platform.h",
        "includes/timer.h"
    ]

    property stringList incPaths: [
        "includes",
        "../../common",
        "../../external/next/inc",
        "../../external/next/inc/math",
        "../../external/next/inc/core",
        "../../external/physfs/inc",
        "../../external/glfw/inc",
    ]

    Application {
        name: "WorldBuilder"
        files: engine.srcFiles
        Depends { name: "cpp" }

        cpp.defines: ["BUILD_SHARED", "NEXT_LIBRARY"]
        cpp.includePaths: engine.incPaths
        cpp.libraryPaths: [
            "../../external-target/libs/next/shared/windows_x32/release",
            "../../external-target/libs/physfs/shared/windows_x32/release",
            "../../external/glfw/lib/lib-vc2013"
        ]
        cpp.dynamicLibraries: [
            "next",
            "physfs",
            "glfw3dll"
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
        name: "engine"
        files: engine.srcFiles
        Depends { name: "cpp" }

        cpp.includePaths: engine.incPaths

        Group {
            name: "Install Static Engine"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: "lib"
            qbs.installPrefix: "../../"
        }
    }
}
