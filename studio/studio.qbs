import qbs

Project {
    id: studio
    property stringList srcFiles: [
        "src/**/*.cpp",
        "src/**/*.h",
        //"res/icon.rc",
        //"res/app-Info.plist"
    ]

    property stringList incPaths: [
        "src",
        "src/screens",
        "../",
        "../editor/includes",
        "../engine/includes",
        "../engine/includes/components",
        "../engine/includes/resources",
        "../modules/uikit/includes",
        "../modules/uikit/includes/components",
        "../modules/uikit/includes/resources",
        "../thirdparty/glfw/include",
        "../thirdparty/next/inc",
        "../thirdparty/next/inc/math",
        "../thirdparty/next/inc/core",
        "../thirdparty/next/inc/anim",
        "../thirdparty/next/inc/os"
    ]

    Application {
        name: studio.STUDIO_NAME
        condition: studio.desktop
        files: studio.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "uikit-editor" }
        Depends { name: "editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets"]; }

        bundle.isBundle: false
        cpp.defines: {
            var result = studio.defines
            result.push("SHARED_DEFINE")
            return result
        }
        cpp.includePaths: studio.incPaths

        property string prefix: qbs.targetOS.contains("windows") ? "lib" : ""
        cpp.dynamicLibraries: [ ]
        cpp.cxxLanguageVersion: studio.languageVersion

        Properties {
            condition: qbs.targetOS.contains("windows")
        }

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@rpath"
            cpp.rpaths: "@executable_path/../Frameworks/"
            Depends { name: "rendermt" }
        }

        Group {
            name: "Install " + studio.BUILDER_NAME
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: studio.BIN_PATH + studio.bundle
            qbs.installPrefix: studio.PREFIX
        }
    }
}
