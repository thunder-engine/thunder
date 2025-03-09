import qbs

Project {
    id: rendermt
    property stringList srcFiles: [
        "src/*.cpp",
        "src/components/*.cpp",
        "src/resources/*.cpp",
    ]

    property stringList incPaths: [
        "includes",
        "../../../common",
        "../../../engine/includes/components",
        "../../../engine/includes/resources",
        "../../../engine/includes",
        "../../../thirdparty/next/inc",
        "../../../thirdparty/next/inc/math",
        "../../../thirdparty/next/inc/core",
        "../../../thirdparty/glfw/include",
        "../../../thirdparty/glfm/include",
        "../../../thirdparty/metal/metal-cpp",
        "../../../thirdparty/metal/metal-cpp-extensions"
    ]

    DynamicLibrary {
        name: "rendermt-editor"
        condition: rendermt.desktop && qbs.targetOS.contains("darwin")
        files: {
            var sources = srcFiles
            sources.push("src/editor/*.cpp")
            sources.push("src/editor/*.mm")
            sources.push("includes/editor/*.h")
            return sources
        }
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "glfw-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }
        bundle.isBundle: false

        cpp.defines: ["SHARED_DEFINE"]
        cpp.includePaths: rendermt.incPaths
        cpp.cxxLanguageVersion: rendermt.languageVersion
        cpp.cxxStandardLibrary: rendermt.standardLibrary
        cpp.minimumMacosVersion: rendermt.osxVersion
        cpp.sonamePrefix: "@executable_path"
        cpp.weakFrameworks: ["Metal", "MetalKit"]
        cpp.objcFlags: ["-fmodules", "-fcxx-modules"]
        cpp.objcxxFlags: ["-fmodules", "-fcxx-modules"]

        Group {
            name: "Install Dynamic RenderMT"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: rendermt.PLUGINS_PATH
            qbs.installPrefix: rendermt.PREFIX
        }

        Group {
            name: "RenderMT includes"
            prefix: "includes/"
            files: [
                "rendermt.h"
            ]
            qbs.install: true
            qbs.installDir: rendermt.INC_PATH + "/modules"
            qbs.installPrefix: rendermt.PREFIX
        }
    }

    StaticLibrary {
        name: "rendermt"
        condition: qbs.targetOS.contains("darwin")
        files: rendermt.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: rendermt.incPaths
        cpp.cxxLanguageVersion: rendermt.languageVersion
        cpp.cxxStandardLibrary: rendermt.standardLibrary
        cpp.minimumMacosVersion: rendermt.osxVersion
        cpp.minimumIosVersion: rendermt.iosVersion
        cpp.minimumTvosVersion: rendermt.tvosVersion
        cpp.debugInformation: true
        cpp.separateDebugInformation: qbs.buildVariant === "release"
        cpp.weakFrameworks: ["Metal"]

        Properties {
            condition: !rendermt.desktop
            cpp.defines: ["THUNDER_MOBILE"]
        }

        Group {
            name: "Install Static RenderMT"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: rendermt.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: rendermt.PREFIX
        }

        Group {
            name: "Debug Symbols"
            fileTagsFilter: qbs.buildVariant === "release" ? ["debuginfo_cl"] : []
            qbs.install: true
            qbs.installDir: rendermt.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/symbols"
            qbs.installPrefix: rendermt.PREFIX
        }
    }
}
