import qbs

Project {
    id: lightbuilder
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/resources/*.cpp",
        "src/tracer/*.cpp",

        "includes/*.h",
        "includes/components/*.h",
        "includes/resources/*.h",
        "includes/tracer/*.h",

        "res/forms/*.ui",

        "src/graph/*.cpp",
        "includes/graph/*.h",
        "src/graph/viewers/*.cpp",
        "includes/graph/viewers/*.h",
    ]

    property stringList incPaths: [
        "includes",
        "../../../common",
        "../../../engine/includes/resources",
        "../../../engine/includes",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/glfw/include",
        "../../../thirdparty/glfm/include",
        "../../../thirdparty/glew/include",
        "../../../thirdparty/glad/include",
    ]

    DynamicLibrary {
        name: "lightbuilder-editor"
        condition: lightbuilder.desktop
        files: lightbuilder.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }
        bundle.isBundle: false

        cpp.defines: ["NEXT_SHARED"]
        cpp.includePaths: lightbuilder.incPaths
        cpp.cxxLanguageVersion: "c++14"
        cpp.minimumMacosVersion: "10.12"
        cpp.cxxStandardLibrary: "libc++"

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["OpenCL"]
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic LightBuilder"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: lightbuilder.PLUGINS_PATH
            qbs.installPrefix: lightbuilder.PREFIX
        }
    }
}
