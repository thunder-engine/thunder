cmake_minimum_required(VERSION 3.10)

project(engine)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/components/*.cpp"
    "src/components/private/*.cpp"
    "src/resources/*.cpp"
    "src/systems/*.cpp"
    "src/filters/*.cpp"
    "src/pipelinetasks/*.cpp"
    "src/utils/*.cpp"
    "src/adapters/platform*.cpp"
)

set(freetypeFiles
    "../thirdparty/freetype/src/base/ftbase.c"
    "../thirdparty/freetype/src/base/ftinit.c"
    "../thirdparty/freetype/src/base/ftsystem.c"
    "../thirdparty/freetype/src/base/ftbbox.c"
    "../thirdparty/freetype/src/base/ftbdf.c"
    "../thirdparty/freetype/src/base/ftbitmap.c"
    "../thirdparty/freetype/src/base/ftcid.c"
    "../thirdparty/freetype/src/base/ftdebug.c"
    "../thirdparty/freetype/src/base/ftfstype.c"
    "../thirdparty/freetype/src/base/ftgasp.c"
    "../thirdparty/freetype/src/base/ftglyph.c"
    "../thirdparty/freetype/src/base/ftgxval.c"
    "../thirdparty/freetype/src/base/ftmm.c"
    "../thirdparty/freetype/src/base/ftotval.c"
    "../thirdparty/freetype/src/base/ftpatent.c"
    "../thirdparty/freetype/src/base/ftpfr.c"
    "../thirdparty/freetype/src/base/ftstroke.c"
    "../thirdparty/freetype/src/base/ftsynth.c"
    "../thirdparty/freetype/src/base/fttype1.c"
    "../thirdparty/freetype/src/base/ftwinfnt.c"
    "../thirdparty/freetype/src/autofit/autofit.c"
    "../thirdparty/freetype/src/bdf/bdf.c"
    "../thirdparty/freetype/src/cff/cff.c"
    "../thirdparty/freetype/src/cache/ftcache.c"
    "../thirdparty/freetype/src/gzip/ftgzip.c"
    "../thirdparty/freetype/src/lzw/ftlzw.c"
    "../thirdparty/freetype/src/pcf/pcf.c"
    "../thirdparty/freetype/src/pfr/pfr.c"
    "../thirdparty/freetype/src/psaux/psaux.c"
    "../thirdparty/freetype/src/pshinter/pshinter.c"
    "../thirdparty/freetype/src/psnames/psmodule.c"
    "../thirdparty/freetype/src/raster/raster.c"
    "../thirdparty/freetype/src/sdf/sdf.c"
    "../thirdparty/freetype/src/sfnt/sfnt.c"
    "../thirdparty/freetype/src/smooth/smooth.c"
    "../thirdparty/freetype/src/svg/svg.c"
    "../thirdparty/freetype/src/truetype/truetype.c"
    "../thirdparty/freetype/src/type1/type1.c"
    "../thirdparty/freetype/src/cid/type1cid.c"
    "../thirdparty/freetype/src/type42/type42.c"
    "../thirdparty/freetype/src/winfonts/winfnt.c"
)

file(GLOB physfs_srcFiles
    "../thirdparty/physfs/src/*.c"
)

set(glfwFiles
    "../thirdparty/glfw/src/context.c"
    "../thirdparty/glfw/src/init.c"
    "../thirdparty/glfw/src/input.c"
    "../thirdparty/glfw/src/monitor.c"
    "../thirdparty/glfw/src/null_init.c"
    "../thirdparty/glfw/src/null_joystick.c"
    "../thirdparty/glfw/src/null_monitor.c"
    "../thirdparty/glfw/src/null_window.c"
    "../thirdparty/glfw/src/platform.c"
    "../thirdparty/glfw/src/vulkan.c"
    "../thirdparty/glfw/src/window.c"
    "../thirdparty/glfw/src/osmesa_context.c"
    "../thirdparty/glfw/src/egl_context.c"
    "../thirdparty/glfw/src/internal.h"
    "../thirdparty/glfw/src/null_platform.h"
    "../thirdparty/glfw/include/GLFW/glfw3.h"
    "../thirdparty/glfw/include/GLFW/glfw3native.h"
)

if (WIN32)
    set(${PROJECT_NAME}_platform
        "../thirdparty/glfw/src/win32_init.c"
        "../thirdparty/glfw/src/win32_joystick.c"
        "../thirdparty/glfw/src/win32_module.c"
        "../thirdparty/glfw/src/win32_monitor.c"
        "../thirdparty/glfw/src/win32_time.c"
        "../thirdparty/glfw/src/win32_thread.c"
        "../thirdparty/glfw/src/win32_window.c"
        "../thirdparty/glfw/src/wgl_context.c"
        "../thirdparty/glfw/src/win32_platform.h"
        "../thirdparty/glfw/src/win32_joystick.h"
    ) 
elseif (APPLE)
    set(${PROJECT_NAME}_platform
        "../thirdparty/glfw/src/cocoa_init.m"
        "../thirdparty/glfw/src/cocoa_joystick.m"
        "../thirdparty/glfw/src/cocoa_monitor.m"
        "../thirdparty/glfw/src/cocoa_time.c"
        "../thirdparty/glfw/src/posix_poll.c"
        "../thirdparty/glfw/src/posix_thread.c"
        "../thirdparty/glfw/src/cocoa_window.m"
        "../thirdparty/glfw/src/nsgl_context.m"
        "../thirdparty/glfw/src/cocoa_platform.h"
        "../thirdparty/glfw/src/cocoa_joystick.h"
    ) 
else ()
    set(${PROJECT_NAME}_platform
        "../thirdparty/glfw/src/x11_init.c"
        "../thirdparty/glfw/src/linux_joystick.c"
        "../thirdparty/glfw/src/xkb_unicode.c"
        "../thirdparty/glfw/src/x11_monitor.c"
        "../thirdparty/glfw/src/posix_module.c"
        "../thirdparty/glfw/src/posix_poll.c"
        "../thirdparty/glfw/src/posix_time.c"
        "../thirdparty/glfw/src/posix_thread.c"
        "../thirdparty/glfw/src/x11_window.c"
        "../thirdparty/glfw/src/glx_context.c"
        "../thirdparty/glfw/src/x11_platform.h"
        "../thirdparty/glfw/src/linux_joystick.h"
    ) 
endif()

set(${PROJECT_NAME}_incPaths
    "../"
    "../thirdparty/next/inc"
    "../thirdparty/next/inc/math"
    "../thirdparty/next/inc/core"
    "../thirdparty/next/inc/anim"
    "../thirdparty/physfs/src"
    "../thirdparty/glfw/include"
    "../thirdparty/glfw/src"
    "../thirdparty/glfm/include"
    "../thirdparty/freetype/include"
    "includes/components"
    "includes/resources"
    "includes/adapters"
    "includes/utils"
    "includes"
)

add_definitions (
    -DCOMPANY_NAME="${COMPANY_NAME}"
    -DEDITOR_NAME="${EDITOR_NAME}"
    -DSDK_VERSION="${SDK_VERSION}"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/.. FILES ${${PROJECT_NAME}_srcFiles} ${freetypeFiles} ${physfs_srcFiles} ${glfwFiles} ${PROJECT_NAME}_platform)

if(desktop)
    # This path is only needed on the BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        set(${PROJECT_NAME}_incPaths
            ${${PROJECT_NAME}_incPaths}
            ${Qt5Gui_INCLUDE_DIRS}
            ${Qt5Svg_INCLUDE_DIRS}
            ${Qt5Widgets_INCLUDE_DIRS}
            "/usr/local/include"
        )
    endif()

    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "includes/editor"
        "../thirdparty/pugixml/src"
    )

    # Dynamic Library
    add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcFiles} ${freetypeFiles} ${physfs_srcFiles} ${glfwFiles} ${${PROJECT_NAME}_platform})

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        assimp
        next-editor
    )

    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
        ENGINE_LIBRARY
        FT2_BUILD_LIBRARY
        PHYSFS_SUPPORTS_ZIP
        PHYSFS_SUPPORTS_DEFAULT=0
        PHYSFS_NO_CDROM_SUPPORT
    )

    if(UNIX AND NOT APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
        )

        # Solve build error using Clang on BSDs
        if(NOT LINUX)
            target_compile_options(${PROJECT_NAME}-editor PRIVATE -fPIC)
        endif()
    endif()

    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    target_sources(${PROJECT_NAME}-editor PRIVATE
        "src/adapters/desktopadaptor.cpp"
    )

    if(WIN32)
		target_compile_definitions(${PROJECT_NAME}-editor PRIVATE _GLFW_WIN32)
		target_link_libraries(${PROJECT_NAME}-editor PRIVATE Shell32 Advapi32 Gdi32 User32)
	elseif (UNIX AND NOT APPLE)
		target_compile_definitions(${PROJECT_NAME}-editor PRIVATE _GLFW_X11)
		target_link_libraries(${PROJECT_NAME}-editor PRIVATE X11 Xrandr Xi Xxf86vm Xcursor Xinerama)
    elseif(APPLE)
		target_compile_definitions(${PROJECT_NAME}-editor PRIVATE _GLFW_COCOA)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE "-framework IOKit" "-framework Foundation" "-framework CoreFoundation" "-framework AppKit" "-framework CoreVideo")
    endif()

    file(GLOB ${PROJECT_NAME}_headers
        "includes/*.h"
        "includes/components/*.h"
        "includes/resources/*.h"
        "includes/pipelinetasks/*.h"
    )

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../${LIB_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}-editor
        DESTINATION "${LIB_PATH}"
    )

    install(FILES ${${PROJECT_NAME}_headers}
        DESTINATION "${INC_PATH}/engine"
    )

    set(${${PROJECT_NAME}_srcFiles}
        ${${PROJECT_NAME}_srcFiles}
        ${glfwFiles}
        ${${PROJECT_NAME}_platform}
    )

endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles} ${freetypeFiles} ${physfs_srcFiles})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    ENGINE_LIBRARY
    FT2_BUILD_LIBRARY
    PHYSFS_SUPPORTS_ZIP
    PHYSFS_SUPPORTS_DEFAULT=0
    PHYSFS_NO_CDROM_SUPPORT
)

if(WIN32)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _GLFW_WIN32)
elseif(UNIX AND NOT APPLE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _GLFW_X11)
elseif(APPLE)
	target_compile_definitions(${PROJECT_NAME} PRIVATE _GLFW_COCOA)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

if(NOT desktop)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        THUNDER_MOBILE
    )

    target_sources(${PROJECT_NAME} PRIVATE
        "src/adapters/mobileadaptor.cpp"
    )
	
    if(EMSCRIPTEN)
        target_sources(${PROJECT_NAME} PRIVATE
            "src/adapters/emscriptenplatform.cpp"
        )
    endif()
endif()

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../${STATIC_PATH}"
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION "${STATIC_PATH}"
)
