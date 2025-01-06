cmake_minimum_required(VERSION 3.10)

project(freetype)

set(srcFiles
    "src/base/ftbase.c"
    "src/base/ftinit.c"
    "src/base/ftsystem.c"
    "src/base/ftbbox.c"
    "src/base/ftbdf.c"
    "src/base/ftbitmap.c"
    "src/base/ftcid.c"
    "src/base/ftdebug.c"
    "src/base/ftfstype.c"
    "src/base/ftgasp.c"
    "src/base/ftglyph.c"
    "src/base/ftgxval.c"
    "src/base/ftmm.c"
    "src/base/ftotval.c"
    "src/base/ftpatent.c"
    "src/base/ftpfr.c"
    "src/base/ftstroke.c"
    "src/base/ftsynth.c"
    "src/base/fttype1.c"
    "src/base/ftwinfnt.c"
    "src/autofit/autofit.c"
    "src/bdf/bdf.c"
    "src/cff/cff.c"
    "src/cache/ftcache.c"
    "src/gzip/ftgzip.c"
    "src/lzw/ftlzw.c"
    "src/pcf/pcf.c"
    "src/pfr/pfr.c"
    "src/psaux/psaux.c"
    "src/pshinter/pshinter.c"
    "src/psnames/psmodule.c"
    "src/raster/raster.c"
    "src/sdf/sdf.c"
    "src/sfnt/sfnt.c"
    "src/smooth/smooth.c"
    "src/svg/svg.c"
    "src/truetype/truetype.c"
    "src/type1/type1.c"
    "src/cid/type1cid.c"
    "src/type42/type42.c"
    "src/winfonts/winfnt.c"
)

set(incPaths
    "include"
    "include/freetype"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(incPaths
        ${incPaths}
        "/usr/local/include"
    )
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${srcFiles})

# Dynamic Library
if(desktop)
    add_library(${PROJECT_NAME}-editor SHARED ${srcFiles})
    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE FT2_BUILD_LIBRARY DLL_EXPORT)
    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${incPaths})
    target_compile_features(${PROJECT_NAME}-editor PRIVATE cxx_std_14)

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${LIB_PATH}"
    )

    if(APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            MACOSX_RPATH TRUE
            VERSION ${PROJECT_VERSION}
        )
        target_compile_options(${PROJECT_NAME}-editor PRIVATE "-Wno-argument-outside-range")
    endif()

    install(TARGETS ${PROJECT_NAME}-editor
            DESTINATION ${LIB_PATH}
            ARCHIVE DESTINATION ${LIB_PATH}
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${srcFiles})
target_compile_definitions(${PROJECT_NAME} PRIVATE FT2_BUILD_LIBRARY)
target_include_directories(${PROJECT_NAME} PRIVATE ${incPaths})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

if(ANDROID)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        ANDROID_NDK_PLATFORM ${ANDROID}
        ANDROID_NDK_ABI ${CMAKE_ANDROID_ARCH_ABI}
        ANDROID_STL ${ANDROID_STL}
    )
endif()

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${STATIC_PATH}
    ARCHIVE DESTINATION ${STATIC_PATH}
)
