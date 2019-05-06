import qbs

Project {
    id: project
    property string platform: {
        var arch = qbs.architecture;
        if(qbs.targetOS[0] === "darwin" || qbs.targetOS[0] === "linux") {
            arch = "x86_64"
        }
        return "/" + qbs.targetOS[0] + "/" + arch;
    }

    property string sdkPath: "${sdkPath}"
    property stringList includePaths: [
        sdkPath + "/include/engine",
        sdkPath + "/include/next",
        sdkPath + "/include/next/math",
        sdkPath + "/include/next/core"
    ]
    property bool desktop: !qbs.targetOS.contains("android") && !qbs.targetOS.contains("ios") && !qbs.targetOS.contains("tvos")
    property bool isAndroid: qbs.targetOS.contains("android")

    DynamicLibrary {
        condition: desktop
        name: "${Project_Name}-Editor"
        files: [ ${filesList},
            "plugin.cpp" ]
        Depends { name: "cpp" }
        cpp.cxxLanguageVersion: "c++14"
        cpp.defines: ["NEXT_SHARED"]
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
            "application.cpp",
            "plugin.cpp" ]
        Depends { name: "cpp" }
        Depends { name: "bundle" }

        bundle.identifierPrefix: "com.thunderengine"
        cpp.cxxLanguageVersion: "c++14"
        cpp.includePaths: project.includePaths
        cpp.libraryPaths: [ ${libraryPaths}
            project.sdkPath + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/lib"
        ]

        cpp.staticLibraries: [
            "engine",
            "next",
            "physfs",
            "freetype",
            "rendergl"
        ]

        Properties {
            condition: desktop
            cpp.staticLibraries: outer.concat([
                "zlib",
                "glfw",
                "glad"
            ])
        }
        Properties {
            condition: !desktop
            cpp.staticLibraries: outer.concat([
                "glfm"
            ])
        }

        Properties {
            condition: qbs.targetOS[0] === "windows"
            cpp.dynamicLibraries: [ "Shell32", "User32", "Gdi32", "Advapi32", "opengl32"
            ]
        }
        Properties {
            condition: qbs.targetOS[0] === "linux"
            cpp.dynamicLibraries: [ "X11", "Xrandr", "Xi", "Xxf86vm", "Xcursor", "Xinerama", "dl", "pthread" ]
        }
        Properties {
            condition: qbs.targetOS[0] === "darwin"
            cpp.weakFrameworks: [ "OpenGL", "Cocoa", "CoreVideo", "IOKit" ]
        }
        Properties {
            condition: qbs.targetOS[0] === "android"
            Android.ndk.appStl: "gnustl_static"
            Android.ndk.platform: "android-21"
            Android.sdk.packageName: "com.${Company_Name}.${Project_Name}"
            Android.sdk.manifestFile: "${manifestFile}"
            Android.sdk.assetsDir: "${assetsPath}"
            Android.sdk.resourcesDir: "${resourceDir}"
            cpp.dynamicLibraries: [ "log", "android", "EGL", "GLESv3", "z" ]
            cpp.defines: outer.concat([ "THUNDER_MOBILE" ])
            Depends { productTypes: ["android.nativelibrary"] }
        }

        Group {
            name: "Install Application"
            qbs.install: true
            qbs.installDir: ""
            qbs.installSourceBase: product.buildDirectory
            qbs.installPrefix: ""

            fileTagsFilter: isForAndroid ? ["android.apk"] : (isBundle ? ["bundle.content"] : ["application"])
        }
    }
}

