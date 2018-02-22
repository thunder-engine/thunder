import qbs

Project {
    id: project
    property string platform: "/windows/x86"
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

        cpp.defines: ["BUILD_SHARED", "NEXT_LIBRARY"]
        cpp.includePaths: project.includePaths
        cpp.libraryPaths: [ ${libraryPaths}
            project.sdkPath + "/bin" + project.platform
        ]
        cpp.dynamicLibraries: [
            "next",
            "engine" ]

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

        cpp.defines: []
        cpp.includePaths: project.includePaths
        cpp.libraryPaths: [ ${libraryPaths}
            project.sdkPath + project.platform + "/lib"
        ]

        cpp.staticLibraries: [
            "next",
            "engine",
            "rendergl",
            "physfs",
            "zlib",
            "glfw",
            "glad",
            "opengl32"
        ]

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.dynamicLibraries: [
                "Shell32",
                "User32",
                "Gdi32",
                "Advapi32"
            ]
            //qbs.debugInformation: true
        }

        Group {
            name: "Install Application"
            fileTagsFilter: "application"
            qbs.install: true
            qbs.installDir: ""
        }
    }
}

