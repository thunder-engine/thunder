cmake_minimum_required(VERSION 3.10)

project(glfw)

set(srcFiles
    "src/context.c"
    "src/init.c"
    "src/input.c"
    "src/monitor.c"
    "src/vulkan.c"
    "src/window.c"
    "src/osmesa_context.c"
    "src/egl_context.c"
    "src/internal.h"
    "include/GLFW/glfw3.h"
    "include/GLFW/glfw3native.h"
)

if (WIN32)
    set(${PROJECT_NAME}_platform
        "src/win32_init.c"
        "src/win32_joystick.c"
        "src/win32_monitor.c"
        "src/win32_time.c"
        "src/win32_thread.c"
        "src/win32_window.c"
        "src/wgl_context.c"
        "src/win32_platform.h"
        "src/win32_joystick.h"
        "src/wgl_context.h"
    ) 
elseif (APPLE)
    set(${PROJECT_NAME}_platform
        "src/cocoa_init.m"
        "src/cocoa_joystick.m"
        "src/cocoa_monitor.m"
        "src/cocoa_time.c"
        "src/posix_thread.c"
        "src/cocoa_window.m"
        "src/nsgl_context.m"
        "src/cocoa_platform.h"
        "src/cocoa_joystick.h"
        "src/nsgl_context.h"
    ) 
else ()
    set(${PROJECT_NAME}_platform
        "src/x11_init.c"
        "src/linux_joystick.c"
        "src/xkb_unicode.c"
        "src/x11_monitor.c"
        "src/posix_time.c"
        "src/posix_thread.c"
        "src/x11_window.c"
        "src/glx_context.c"
        "src/x11_platform.h"
        "src/linux_joystick.h"
        "src/glx_context.h"
    ) 
endif()

set(incPaths "include")

# Dynamic Library
if(desktop)
    add_library(${PROJECT_NAME}-editor SHARED ${srcFiles} ${${PROJECT_NAME}_platform})
    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE _GLFW_BUILD_DLL)
    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${incPaths})
    target_compile_features(${PROJECT_NAME}-editor PRIVATE c_std_11)

    # Windows-specific configurations
    if(WIN32)
        target_compile_definitions(${PROJECT_NAME}-editor PRIVATE _GLFW_WIN32)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE gdi32 User32 Shell32)
    endif()

    # Linux-specific configurations
    if(UNIX AND NOT APPLE)
        target_compile_definitions(${PROJECT_NAME}-editor PRIVATE _GLFW_X11)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE X11 Xrandr Xi Xxf86vm Xcursor Xinerama)
    endif()

    # macOS-specific configurations
    if(APPLE)
        target_compile_definitions(${PROJECT_NAME}-editor PRIVATE _GLFW_COCOA)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE "-framework CoreFoundation" "-framework AppKit" "-framework CoreVideo" "-framework IOKit")
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            MACOSX_RPATH TRUE
            SUFFIX ".dylib"
        )
    endif()

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${LIB_PATH}"
    )

    # Install
    install(TARGETS ${PROJECT_NAME}-editor
            LIBRARY DESTINATION ${LIB_PATH}
            ARCHIVE DESTINATION ${LIB_PATH}
            RUNTIME DESTINATION ${LIB_PATH}
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${srcFiles} ${${PROJECT_NAME}_platform})
target_compile_features(${PROJECT_NAME} PRIVATE c_std_11)
target_include_directories(${PROJECT_NAME} PRIVATE ${incPaths})

# Windows-specific configurations
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _GLFW_WIN32)
endif()

# Linux-specific configurations
if(UNIX AND NOT APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _GLFW_X11)
endif()

# macOS-specific configurations
if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _GLFW_COCOA)
endif()

# Install
install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${STATIC_PATH}
        ARCHIVE DESTINATION ${STATIC_PATH}
)
