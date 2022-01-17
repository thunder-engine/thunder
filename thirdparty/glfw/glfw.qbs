import qbs
import qbs.FileInfo

Project {
    id: glfw
    property stringList srcFiles: {
        var sources = [
        "src/context.c",
        "src/init.c",
        "src/input.c",
        "src/monitor.c",
        "src/vulkan.c",
        "src/window.c",
        "src/osmesa_context.c",

        "src/internal.h",
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h"
        ];
        if(qbs.targetOS.contains("windows")) {
            sources.push("src/win32_init.c"),
            sources.push("src/win32_joystick.c"),
            sources.push("src/win32_monitor.c"),
            sources.push("src/win32_time.c"),
            sources.push("src/win32_thread.c"),
            sources.push("src/win32_window.c"),
            sources.push("src/wgl_context.c"),
            sources.push("src/egl_context.c"),

            sources.push("src/win32_platform.h"),
            sources.push("src/win32_joystick.h"),
            sources.push("src/wgl_context.h"),
            sources.push("src/egl_context.h")
        } else if(qbs.targetOS.contains("darwin")) {
            sources.push("src/cocoa_init.m"),
            sources.push("src/cocoa_joystick.m"),
            sources.push("src/cocoa_monitor.m"),
            sources.push("src/cocoa_time.c"),
            sources.push("src/posix_thread.c"),
            sources.push("src/cocoa_window.m"),
            sources.push("src/nsgl_context.m"),

            sources.push("src/cocoa_platform.h"),
            sources.push("src/cocoa_joystick.h"),
            sources.push("src/nsgl_context.h")
        } else if(qbs.targetOS.contains("linux")) {
            sources.push("src/x11_init.c"),
            sources.push("src/linux_joystick.c"),
            sources.push("src/xkb_unicode.c"),
            sources.push("src/x11_monitor.c"),
            sources.push("src/posix_time.c"),
            sources.push("src/posix_thread.c"),
            sources.push("src/x11_window.c"),
            sources.push("src/glx_context.c"),
            sources.push("src/egl_context.c"),

            sources.push("src/x11_platform.h"),
            sources.push("src/linux_joystick.h"),
            sources.push("src/glx_context.h"),
            sources.push("src/egl_context.h")
        }

        return sources;
    }

    property stringList incPaths: [
        "include"
    ]

    DynamicLibrary {
        name: "glfw-editor"
        condition: glfw.desktop
        files: glfw.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.defines: ["_GLFW_BUILD_DLL"]
        cpp.includePaths: glfw.incPaths
        cpp.libraryPaths: [ ]

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.defines: outer.concat(["_GLFW_WIN32"])
            cpp.dynamicLibraries: [ "gdi32", "User32", "Shell32" ]
        }
		
		Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.defines: outer.concat(["_GLFW_X11"])
            cpp.dynamicLibraries: [ "X11", "Xrandr", "Xi", "Xxf86vm", "Xcursor", "Xinerama" ]
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.defines: outer.concat(["_GLFW_COCOA"])
            cpp.weakFrameworks: [ "CoreFoundation", "AppKit", "CoreVideo", "IOKit" ]
            cpp.sonamePrefix: "@executable_path"
        }

        Group {
            name: "Install Dynamic glfw"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: glfw.LIB_PATH + "/" + glfw.bundle
            qbs.installPrefix: glfw.PREFIX
        }
    }

    StaticLibrary {
        name: "glfw"
        condition: glfw.desktop
        files: glfw.srcFiles
        Depends { name: "cpp" }
        Depends { name: "bundle" }
        bundle.isBundle: false

        cpp.includePaths: glfw.incPaths

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.defines: ["_GLFW_WIN32"]
        }
		
		Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.defines: outer.concat(["_GLFW_X11"])
        }

        Properties {
            condition: qbs.targetOS.contains("darwin")
            cpp.defines: outer.concat(["_GLFW_COCOA"])
        }

        Group {
            name: "Install Static glfw"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: glfw.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
            qbs.installPrefix: glfw.PREFIX
        }
    }
}
