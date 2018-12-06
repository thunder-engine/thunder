import qbs

Project {
    id: project
    property string platform: {
        if(qbs.targetOS.contains("windows")) {
            return "/windows/x86";
        }
        if(qbs.targetOS.contains("linux")) {
            return "/linux/x86_64"
        }

        return "/macos/x86_64";
    }
    property string sdkPath: "${sdkPath}"
    property stringList includePaths: [
        sdkPath + "/include/engine",
        sdkPath + "/include/next",
        sdkPath + "/include/next/math",
        sdkPath + "/include/next/core"
    ]

    DynamicLibrary {
        name: "${Project_Name}-Editor"
        files: [ ${filesList},
            "plugin.cpp" ]
        Depends { name: "cpp" }

        cpp.defines: ["NEXT_SHARED", "NEXT_LIBRARY"]
        cpp.includePaths: project.includePaths
        cpp.libraryPaths: [ ${libraryPaths}
            project.sdkPath + project.platform + "/bin"
        ]
        cpp.dynamicLibraries: [
            "next-editor",
            "engine-editor" ]

        Group {
            name: "Install Plugin"
            fileTagsFilter: "dynamiclibrary"
            qbs.install: true
            qbs.installDir: ""
        }
    }

    Application {
        name: "${Project_Name}"
        consoleApplication: false

        files: [ ${filesList},
            "application.cpp" ]
        Depends { name: "cpp" }

        property bool isBundle: qbs.targetOS.contains("darwin") && bundle.isBundle
        bundle.identifierPrefix: "com.thunderengine"

        cpp.defines: ["NEXT_LIBRARY"]
        cpp.includePaths: project.includePaths
        cpp.libraryPaths: [ ${libraryPaths}
            project.sdkPath + project.platform + "/lib"
        ]

        cpp.staticLibraries: [
            "engine",
            "next",
            "physfs",
            "freetype",
            "zlib",
            "glfw",
            "rendergl",
            "glad"
        ]

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.dynamicLibraries: [
                "Shell32",
                "User32",
                "Gdi32",
                "Advapi32",
                "opengl32"
            ]
        }
        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.dynamicLibraries: [ "X11", "Xrandr", "Xi", "Xxf86vm", "Xcursor", "Xinerama", "dl", "pthread" ]
        }
        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.weakFrameworks: ["OpenGL", "Cocoa", "CoreVideo", "IOKit"]
        }

        Group {
            name: "Install Application"
            qbs.install: true
            qbs.installDir: ""

            fileTagsFilter: isBundle ? ["bundle.content"] : ["application"]
            qbs.installSourceBase: product.buildDirectory
        }
    }
}

