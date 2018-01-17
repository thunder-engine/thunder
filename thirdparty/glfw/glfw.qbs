import qbs

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

        "src/internal.h",
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h"
        ];
        if(qbs.targetOS.contains("windows")) {
            sources.push("src/win32_init.c"),
            sources.push("src/win32_joystick.c"),
            sources.push("src/win32_monitor.c"),
            sources.push("src/win32_time.c"),
            sources.push("src/win32_tls.c"),
            sources.push("src/win32_window.c"),
            sources.push("src/wgl_context.c"),
            sources.push("src/egl_context.c"),

            sources.push("src/win32_platform.h"),
            sources.push("src/win32_joystick.h"),
            sources.push("src/wgl_context.h"),
            sources.push("src/egl_context.h")
        }
        return sources;
    }

    property stringList incPaths: [
        "include"
    ]

    DynamicLibrary {
        name: "glfw-editor"
        files: glfw.srcFiles
        Depends { name: "cpp" }

        cpp.defines: ["_GLFW_WIN32", "_GLFW_BUILD_DLL"]

        cpp.includePaths: glfw.incPaths
        cpp.libraryPaths: [ ]
        cpp.dynamicLibraries: [ "gdi32", "User32", "Shell32" ]

        Group {
            name: "Install Dynamic glfw"
            fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
            qbs.install: true
            qbs.installDir: glfw.BIN_PATH
            qbs.installPrefix: glfw.PREFIX
        }
    }

    StaticLibrary {
        name: "glfw"
        files: glfw.srcFiles
        Depends { name: "cpp" }

        cpp.defines: ["_GLFW_WIN32"]
        cpp.includePaths: glfw.incPaths

        Group {
            name: "Install Static glfw"
            fileTagsFilter: product.type
            qbs.install: true
            qbs.installDir: glfw.LIB_PATH
            qbs.installPrefix: glfw.PREFIX
        }
    }
}
