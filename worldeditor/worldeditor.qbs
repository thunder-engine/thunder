import qbs

Project {
    id: worldEditor
    property stringList srcFiles: [
        "src/**/*.ui",
        "src/**/*.cpp",
        "src/**/*.h",
        "res/icon.rc",
        "res/WorldEditor.qrc",
        "res/app-Info.plist"
    ]

    property stringList incPaths: [
        "src",
        "../",
        "../engine/includes",
        "../engine/includes/resources",
        "../engine/includes/components",
        "../thirdparty/next/inc",
        "../thirdparty/next/inc/math",
        "../thirdparty/next/inc/core",
        "../thirdparty/next/inc/anim"
    ]

    QtGuiApplication {
        name: worldEditor.EDITOR_NAME
        condition: worldEditor.desktop
        files: worldEditor.srcFiles

        Depends { name: "cpp" }
        Depends { name: "bundle" }
        Depends { name: "next-editor" }
        Depends { name: "engine-editor" }
        Depends { name: "Qt"; submodules: ["core", "gui", "widgets", "xml", "quickwidgets"]; }

        property bool isBundle: qbs.targetOS.contains("darwin") && bundle.isBundle
        bundle.infoPlist: ({
            "NSHumanReadableCopyright": "(C) 2007-" + worldEditor.COPYRIGHT_YEAR + " by " + worldEditor.COPYRIGHT_AUTHOR
        })
        bundle.identifierPrefix: "com.thunderengine"

        consoleApplication: false

        cpp.defines: {
            var result = worldEditor.defines
            result.push("SHARED_DEFINE")
            return result
        }

        cpp.includePaths: worldEditor.incPaths
        property string prefix: qbs.targetOS.contains("windows") ? "lib" : ""
        cpp.cxxLanguageVersion: worldEditor.languageVersion

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.rpaths: "$ORIGIN/../lib"
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.sonamePrefix: "@rpath"
            cpp.rpaths: "@executable_path/../Frameworks/"
        }

        Group {
            name: "Install " + worldEditor.EDITOR_NAME
            qbs.install: true
            qbs.installDir: worldEditor.BIN_PATH
            qbs.installPrefix: worldEditor.PREFIX

            fileTagsFilter: isBundle ? ["bundle.content"] : ["application"]
            qbs.installSourceBase: product.buildDirectory
        }

        Group {
            name: "Icon"
            qbs.install: qbs.targetOS.contains("darwin")
            files: [
                "res/icons/thunder.icns",
            ]
            qbs.installDir: worldEditor.BIN_PATH + "/../Resources"
            qbs.installPrefix: worldEditor.PREFIX
        }
    }
}
