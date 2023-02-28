# import qbs
# 
# Project {
#     id: project
#     property string platform: {
#         var arch = qbs.architecture;
#         if(qbs.targetOS[0] === "macos" || qbs.targetOS[0] === "linux") {
#             arch = "x86_64"
#         } else if(qbs.targetOS[0] === "ios") {
#             //arch = "arm64"
#         }
#         return "/" + qbs.targetOS[0] + "/" + arch;
#     }
# 
#     property string sdkPath: "${sdkPath}"
#     property stringList includePaths: [
#         sdkPath + "/include/engine",
#         sdkPath + "/include/modules",
#         sdkPath + "/include/next",
#         sdkPath + "/include/next/math",
#         sdkPath + "/include/next/core"
#     ]
#     property bool desktop: !qbs.targetOS.contains("android") && !qbs.targetOS.contains("ios") && !qbs.targetOS.contains("tvos")
#     property bool isAndroid: qbs.targetOS.contains("android")
#     property bool isBundle: qbs.targetOS.contains("darwin")
# 
#     DynamicLibrary {
#         condition: desktop
#         name: "${Project_Name}-editor"
#         files: [
#             "plugin.cpp",
#             //+{FilesList}
#             //-{FilesList}
#         ]
#         Depends { name: "cpp" }
#         cpp.cxxLanguageVersion: "c++14"
#         cpp.defines: ["SHARED_DEFINE"]
#         cpp.includePaths: project.includePaths
#         cpp.libraryPaths: [ ${libraryPaths}
#             project.sdkPath + project.platform + "/lib",
#             project.sdkPath + project.platform + "/bin",
#             project.sdkPath + project.platform + "/bin/plugins"
#         ]
#         cpp.dynamicLibraries: [
#             //+{EditorLibrariesList}
#             //-{EditorLibrariesList}
#             "next-editor",
#             "engine-editor"
#         ]
# 
#         Group {
#             name: "Install Plugin"
#             fileTagsFilter: "dynamiclibrary"
#             qbs.install: true
#             qbs.installDir: ""
#             qbs.installPrefix: ""
#         }
#     }
# 
#     Application {
#         name: "${Project_Name}"
#         consoleApplication: false
# 
#         files: [
#             "application.cpp",
#             "plugin.cpp",
#             //+{FilesList}
#             //-{FilesList}
#         ]
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
# 
#         bundle.identifierPrefix: "${Identifier_Prefix}"
#         cpp.cxxLanguageVersion: "c++14"
#         cpp.includePaths: project.includePaths
#         cpp.libraryPaths: [ ${libraryPaths}
#             project.sdkPath + project.platform + "/static"
#         ]
# 
#         cpp.staticLibraries: [
#             //+{LibrariesList}
#             //-{LibrariesList}
#             "engine",
#             "next",
#             "physfs",
#             "freetype",
#             "angelscript",
#             "bullet3"
#         ]
# 
#         Properties {
#             condition: desktop
#             cpp.staticLibraries: outer.concat([
#                 "zlib",
#                 "glfw",
#                 "glad"
#             ])
#         }
#         Properties {
#             condition: !desktop
#             cpp.staticLibraries: outer.concat([
#                 "glfm"
#             ])
#         }
# 
#         Properties {
#             condition: qbs.targetOS[0] === "windows"
#             cpp.dynamicLibraries: [ "Shell32", "User32", "Gdi32", "Advapi32", "opengl32" ]
#         }
#         Properties {
#             condition: qbs.targetOS[0] === "linux"
#             cpp.dynamicLibraries: [ "X11", "Xrandr", "Xi", "Xxf86vm", "Xcursor", "Xinerama", "dl", "pthread" ]
#         }
#         Properties {
#             condition: qbs.targetOS[0] === "macos"
#             cpp.weakFrameworks: [ "OpenGL", "Cocoa", "CoreVideo", "IOKit" ]
#         }
#         Properties {
#             condition: qbs.targetOS[0] === "ios"
#             cpp.weakFrameworks: [ "OpenGLES", "UIKit", "CoreGraphics", "Foundation", "QuartzCore" ]
#             cpp.dynamicLibraries: [ "z" ]
#             cpp.defines: outer.concat([ "THUNDER_MOBILE" ])
#             cpp.minimumIosVersion: "10.0"
#         }
#         Properties {
#             condition: qbs.targetOS[0] === "tvos"
#             cpp.weakFrameworks: [ "OpenGLES", "UIKit", "CoreGraphics", "Foundation", "QuartzCore" ]
#             cpp.dynamicLibraries: [ "z" ]
#             cpp.defines: outer.concat([ "THUNDER_MOBILE" ])
#             cpp.minimumTvosVersion: "10.0"
#         }
#         Properties {
#             condition: qbs.targetOS[0] === "android"
#             Android.ndk.appStl: "c++_shared"
#             Android.ndk.platform: "android-21"
#             Android.sdk.packageName: "com.${Company_Name}.${Project_Name}"
#             Android.sdk.manifestFile: "${manifestFile}"
#             Android.sdk.assetsDir: "${assetsPath}"
#             Android.sdk.resourcesDir: "${resourceDir}"
#             cpp.dynamicLibraries: [ "log", "android", "EGL", "GLESv3", "z" ]
#             cpp.defines: outer.concat([ "THUNDER_MOBILE" ])
#             cpp.cxxStandardLibrary: "libc++"
#             Depends { productTypes: ["android.nativelibrary"] }
#         }
# 
#         Group {
#             name: "Install Application"
#             qbs.install: true
#             qbs.installDir: ""
#             qbs.installSourceBase: product.buildDirectory
#             qbs.installPrefix: ""
# 
#             fileTagsFilter: isForAndroid ? ["android.apk"] : (project.isBundle ? ["bundle.content"] : ["application"])
#         }
#     }
# }
