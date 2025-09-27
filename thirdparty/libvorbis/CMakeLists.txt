cmake_minimum_required(VERSION 3.10)

project(vorbis)

# Vorbis Library
set(vorbis_src_files
    "src/mapping0.c"
    "src/lsp.c"
    "src/lpc.c"
    "src/lookup.c"
    "src/info.c"
    "src/floor1.c"
    "src/floor0.c"
    "src/envelope.c"
    "src/codebook.c"
    "src/block.c"
    "src/bitrate.c"
    "src/analysis.c"
    "src/window.c"
    "src/vorbisenc.c"
    "src/synthesis.c"
    "src/smallft.c"
    "src/sharedbook.c"
    "src/res0.c"
    "src/registry.c"
    "src/psy.c"
    "src/mdct.c"
)

set(${PROJECT_NAME}_incPaths
    "src"
    "../libogg/src"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

if(WIN32 AND NOT MINGW)
    set(vorbis_def src/vorbis.def)
endif()

# Dynamic Library
if(desktop)
    add_library(${PROJECT_NAME}-editor SHARED ${vorbis_src_files} ${vorbis_def})
    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths})
    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE LIBVORBIS_LIBRARY)

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        ogg-editor
    )

    if (APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            INSTALL_NAME_DIR "@executable_path"
        )
    endif()

    if(UNIX AND NOT APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
        )

        # Solve build error using Clang on BSDs
        if(NOT LINUX)
            target_compile_options(${PROJECT_NAME}-editor PRIVATE -fPIC)
        endif()
    endif()

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${LIB_PATH}"
        FOLDER "thirdparty"
    )

    # Install Vorbis Dynamic Library
    install(TARGETS ${PROJECT_NAME}-editor
        LIBRARY DESTINATION ${LIB_PATH}
        ARCHIVE DESTINATION ${LIB_PATH}
        RUNTIME DESTINATION ${LIB_PATH}
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${vorbis_src_files})
target_include_directories(${PROJECT_NAME} PRIVATE "src" "../libogg/src")

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
    FOLDER "thirdparty"
)

# Install Vorbis Library
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${STATIC_PATH}
    ARCHIVE DESTINATION ${STATIC_PATH}
)

if(desktop)
    # VorbisFile Dynamic Library
    add_library(${PROJECT_NAME}file-editor SHARED "src/vorbisfile.c")
    target_include_directories(${PROJECT_NAME}file-editor PRIVATE "../libogg/src")
    target_compile_definitions(${PROJECT_NAME}file-editor PRIVATE LIBVORBIS_LIBRARY)

    if(MSVC)
        target_sources(${PROJECT_NAME}file-editor PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/vorbisfile.def #
        )
    endif ()

    target_link_libraries(${PROJECT_NAME}file-editor PRIVATE
        ogg-editor
        vorbis-editor
    )

    if(APPLE)
        set_target_properties(${PROJECT_NAME}file-editor PROPERTIES
            INSTALL_NAME_DIR "@executable_path"
        )
    endif()

    if(UNIX AND NOT APPLE)
        set_target_properties(${PROJECT_NAME}file-editor PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib"
        )

        # Solve build error using Clang on BSDs
        if(NOT LINUX)
            target_compile_options(${PROJECT_NAME}file-editor PRIVATE -fPIC)
        endif()
    endif()

    set_target_properties(${PROJECT_NAME}file-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${LIB_PATH}"
        FOLDER "thirdparty"
    )

    # Install Vorbis Dynamic Library
    install(TARGETS ${PROJECT_NAME}file-editor
            LIBRARY DESTINATION ${LIB_PATH}
            ARCHIVE DESTINATION ${LIB_PATH}
            RUNTIME DESTINATION ${LIB_PATH}
    )
endif()

# VorbisFile Static Library
add_library(${PROJECT_NAME}file STATIC "src/vorbisfile.c")
target_include_directories(${PROJECT_NAME}file PRIVATE "../libogg/src")
target_compile_definitions(${PROJECT_NAME}file PRIVATE LIBVORBIS_LIBRARY)

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

set_target_properties(${PROJECT_NAME}file PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
    FOLDER "thirdparty"
)

# Install VorbisFile Library
install(TARGETS ${PROJECT_NAME}file
    LIBRARY DESTINATION ${STATIC_PATH}
    ARCHIVE DESTINATION ${STATIC_PATH}
)
