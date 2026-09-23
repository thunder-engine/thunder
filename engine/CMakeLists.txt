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
    "../thirdparty/physfs/src/*.c"
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

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles} ${freetypeFiles})

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
    add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcFiles})

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        assimp
        next-editor
        glfw-editor
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
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE Shell32)
    elseif(APPLE)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE "-framework IOKit")
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

endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_compile_definitions(${PROJECT_NAME} PRIVATE
    NEXT_LIBRARY
    FT2_BUILD_LIBRARY
    PHYSFS_SUPPORTS_ZIP
    PHYSFS_SUPPORTS_DEFAULT=0
    PHYSFS_NO_CDROM_SUPPORT
)

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
