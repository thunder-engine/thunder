# import qbs
# import qbs.FileInfo
# 
# Project {
#     id: glfw
#     property stringList srcFiles: {
#         var sources = [
#         "src/context.c",
#         "src/init.c",
#         "src/input.c",
#         "src/monitor.c",
#         "src/vulkan.c",
#         "src/window.c",
#         "src/osmesa_context.c",
#         "src/egl_context.c",
# 
#         "src/internal.h",
#         "include/GLFW/glfw3.h",
#         "include/GLFW/glfw3native.h"
#         ];
#         if(qbs.targetOS.contains("windows")) {
#             sources.push("src/win32_init.c"),
#             sources.push("src/win32_joystick.c"),
#             sources.push("src/win32_monitor.c"),
#             sources.push("src/win32_time.c"),
#             sources.push("src/win32_thread.c"),
#             sources.push("src/win32_window.c"),
#             sources.push("src/wgl_context.c"),
# 
#             sources.push("src/win32_platform.h"),
#             sources.push("src/win32_joystick.h"),
#             sources.push("src/wgl_context.h")
#         } else if(qbs.targetOS.contains("darwin")) {
#             sources.push("src/cocoa_init.m"),
#             sources.push("src/cocoa_joystick.m"),
#             sources.push("src/cocoa_monitor.m"),
#             sources.push("src/cocoa_time.c"),
#             sources.push("src/posix_thread.c"),
#             sources.push("src/cocoa_window.m"),
#             sources.push("src/nsgl_context.m"),
# 
#             sources.push("src/cocoa_platform.h"),
#             sources.push("src/cocoa_joystick.h"),
#             sources.push("src/nsgl_context.h")
#         } else if(qbs.targetOS.contains("linux")) {
#             sources.push("src/x11_init.c"),
#             sources.push("src/linux_joystick.c"),
#             sources.push("src/xkb_unicode.c"),
#             sources.push("src/x11_monitor.c"),
#             sources.push("src/posix_time.c"),
#             sources.push("src/posix_thread.c"),
#             sources.push("src/x11_window.c"),
#             sources.push("src/glx_context.c"),
# 
#             sources.push("src/x11_platform.h"),
#             sources.push("src/linux_joystick.h"),
#             sources.push("src/glx_context.h")
#         }
# 
#         return sources;
#     }
# 
#     property stringList incPaths: [
#         "include"
#     ]
# 
#     DynamicLibrary {
#         name: "glfw-editor"
#         condition: glfw.desktop
#         files: glfw.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.defines: ["_GLFW_BUILD_DLL"]
#         cpp.includePaths: glfw.incPaths
#         cpp.libraryPaths: [ ]
# 
#         Properties {
#             condition: qbs.targetOS.contains("windows")
#             cpp.defines: outer.concat(["_GLFW_WIN32"])
#             cpp.dynamicLibraries: [ "gdi32", "User32", "Shell32" ]
#         }
# 		
# 		Properties {
#             condition: qbs.targetOS.contains("linux")
#             cpp.defines: outer.concat(["_GLFW_X11"])
#             cpp.dynamicLibraries: [ "X11", "Xrandr", "Xi", "Xxf86vm", "Xcursor", "Xinerama" ]
#         }
# 
#         Properties {
#             condition: qbs.targetOS.contains("darwin")
#             cpp.defines: outer.concat(["_GLFW_COCOA"])
#             cpp.weakFrameworks: [ "CoreFoundation", "AppKit", "CoreVideo", "IOKit" ]
#             cpp.sonamePrefix: "@executable_path"
#         }
# 
#         Group {
#             name: "Install Dynamic glfw"
#             fileTagsFilter: ["dynamiclibrary", "dynamiclibrary_import"]
#             qbs.install: true
#             qbs.installDir: glfw.LIB_PATH + "/" + glfw.bundle
#             qbs.installPrefix: glfw.PREFIX
#         }
#     }
# 
#     StaticLibrary {
#         name: "glfw"
#         condition: glfw.desktop
#         files: glfw.srcFiles
#         Depends { name: "cpp" }
#         Depends { name: "bundle" }
#         bundle.isBundle: false
# 
#         cpp.includePaths: glfw.incPaths
# 
#         Properties {
#             condition: qbs.targetOS.contains("windows")
#             cpp.defines: ["_GLFW_WIN32"]
#         }
# 		
# 		Properties {
#             condition: qbs.targetOS.contains("linux")
#             cpp.defines: outer.concat(["_GLFW_X11"])
#         }
# 
#         Properties {
#             condition: qbs.targetOS.contains("darwin")
#             cpp.defines: outer.concat(["_GLFW_COCOA"])
#         }
# 
#         Group {
#             name: "Install Static glfw"
#             fileTagsFilter: product.type
#             qbs.install: true
#             qbs.installDir: glfw.SDK_PATH + "/" + qbs.targetOS[0] + "/" + qbs.architecture + "/static"
#             qbs.installPrefix: glfw.PREFIX
#         }
#     }
# }


project(glfw)

if (NOT CMAKE_SYSTEM_NAME MATCHES "Windows|Linux|Darwin")
    message(-- Skipp ${PROJECT_NAME} for ${CMAKE_SYSTEM_NAME})
    return()
endif()

set(${PROJECT_NAME}_srcList 
    src/context.c
    src/init.c
    src/input.c
    src/monitor.c
    src/vulkan.c
    src/window.c
    src/osmesa_context.c
    src/egl_context.c
    src/internal.h
    include/GLFW/glfw3.h
    include/GLFW/glfw3native.h
)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    list(APPEND ${PROJECT_NAME}_srcList
        src/win32_init.c
        src/win32_joystick.c
        src/win32_monitor.c
        src/win32_time.c
        src/win32_thread.c
        src/win32_window.c
        src/wgl_context.c
        src/win32_platform.h
        src/win32_joystick.h
        src/wgl_context.h
    )
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    list(APPEND ${PROJECT_NAME}_srcList
        src/x11_init.c
        src/linux_joystick.c
        src/xkb_unicode.c
        src/x11_monitor.c
        src/posix_time.c
        src/posix_thread.c
        src/x11_window.c
        src/glx_context.c
        src/x11_platform.h
        src/linux_joystick.h
        src/glx_context.h
    )
endif()



if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    list(APPEND ${PROJECT_NAME}_srcList 
        src/cocoa_init.m
        src/cocoa_joystick.m
        src/cocoa_monitor.m
        src/cocoa_time.c
        src/posix_thread.c
        src/cocoa_window.m
        src/nsgl_context.m
        src/cocoa_platform.h
        src/cocoa_joystick.h
        src/nsgl_context.h
    )
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(${PROJECT_NAME}_defines _GLFW_WIN32)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(${PROJECT_NAME}_defines _GLFW_X11)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(${PROJECT_NAME}_defines _GLFW_COCOA)
endif()


add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME}-editor PUBLIC include)
target_compile_definitions(${PROJECT_NAME}-editor PUBLIC ${${PROJECT_NAME}_defines})


add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcList})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_compile_definitions(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_defines})

