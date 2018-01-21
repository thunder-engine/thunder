import qbs

Project {
    id: engine
    property stringList srcFiles: [
        "src/*.cpp",
        "src/analytics/*.cpp",
        "src/components/*.cpp",
        "src/resources/*.cpp",

        "includes/*.h",
        "includes/analytics/*.h",
        "includes/adapters/*.h",
        "includes/components/*.h",
        "includes/resources/*.h",
    ]

    property stringList incPaths: [
        "includes",
        "../common",
        "../thirdparty/next/inc",
        "../thirdparty/next/inc/math",
        "../thirdparty/next/inc/core",
        "../thirdparty/physfs/src",
        "../thirdparty/glfw/include",
        "../thirdparty/glfm/include"
    ]

    DynamicLibrary {
        name: "engine-editor"
        condition: engine.desktop
        files: engine.srcFiles
        Depends { name: "cpp" }
        Depends { name: "next-editor" }
        Depends { name: "glfw-editor" }
        Depends { name: "physfs-editor" }

        cpp.defines: ["BUILD_SHARED", "NEXT_LIBRARY"]
        cpp.includePaths: engine.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ ]

        Properties {
            condition: engine.desktop
            files: outer.concat(["src/adapters/desktopadaptor.cpp"])
        }

        Group {
            name: "Install Dynamic Engine"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: engine.BIN_PATH
            qbs.installPrefix: engine.PREFIX
        }
    }

    StaticLibrary {
        name: "engine"
        files: engine.srcFiles
        Depends { name: "cpp" }

        cpp.includePaths: engine.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.defines: ["NEXT_LIBRARY"]

        Properties {
            condition: engine.desktop
            files: outer.concat(["src/adapters/desktopadaptor.cpp"])
        }
        Properties {
            condition: !engine.desktop
            files: outer.concat(["src/adapters/mobileadaptor.cpp"])
        }

        Properties {
            condition: qbs.targetOS.contains("android")
            Android.ndk.appStl: "gnustl_shared"
        }

        Group {
            name: "Install Static Engine"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: engine.LIB_PATH
            qbs.installPrefix: engine.PREFIX
        }
    }
}
