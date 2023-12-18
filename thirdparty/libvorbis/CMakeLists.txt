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

if(WIN32 AND NOT MINGW)
    set(vorbis_def src/vorbis.def)
endif()

# Dynamic Library
add_library(${PROJECT_NAME}-editor SHARED ${vorbis_src_files} ${vorbis_def})
target_include_directories(${PROJECT_NAME}-editor PRIVATE "src" "../libogg/src")
target_compile_definitions(${PROJECT_NAME}-editor PRIVATE LIBVORBIS_LIBRARY)

target_link_libraries(${PROJECT_NAME}-editor PRIVATE
    ogg-editor
)

if (APPLE)
    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        INSTALL_NAME_DIR "@executable_path"
    )
endif()

set_target_properties(${PROJECT_NAME}-editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${LIB_PATH}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${LIB_PATH}"
)

# Install Vorbis Dynamic Library
install(TARGETS ${PROJECT_NAME}-editor
        LIBRARY DESTINATION ${LIB_PATH}
        ARCHIVE DESTINATION ${LIB_PATH}
        RUNTIME DESTINATION ${LIB_PATH}
)

# Static Library
add_library(${PROJECT_NAME} STATIC ${vorbis_src_files})
target_include_directories(${PROJECT_NAME} PRIVATE "src" "../libogg/src")

# Install Vorbis Library
install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${STATIC_PATH}
        ARCHIVE DESTINATION ${STATIC_PATH}
)

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

if (APPLE)
    set_target_properties(${PROJECT_NAME}file-editor PROPERTIES
        INSTALL_NAME_DIR "@executable_path"
    )
endif()

set_target_properties(${PROJECT_NAME}file-editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${LIB_PATH}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${LIB_PATH}"
)

# Install Vorbis Dynamic Library
install(TARGETS ${PROJECT_NAME}file-editor
        LIBRARY DESTINATION ${LIB_PATH}
        ARCHIVE DESTINATION ${LIB_PATH}
        RUNTIME DESTINATION ${LIB_PATH}
)

# VorbisFile Static Library
add_library(${PROJECT_NAME}file STATIC "src/vorbisfile.c")
target_include_directories(${PROJECT_NAME}file PRIVATE "../libogg/src")
target_compile_definitions(${PROJECT_NAME}file PRIVATE LIBVORBIS_LIBRARY)

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
)

# Install VorbisFile Library
install(TARGETS ${PROJECT_NAME}file
        LIBRARY DESTINATION ${STATIC_PATH}
        ARCHIVE DESTINATION ${STATIC_PATH}
)
