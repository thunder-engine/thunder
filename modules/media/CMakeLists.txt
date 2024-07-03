cmake_minimum_required(VERSION 3.10)

project(media)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/components/*.cpp"
    "src/resources/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "includes"
    "../../common"
    "../../engine/includes"
    "../../engine/includes/resources"
    "../../engine/includes/components"
    "../../engine/includes/editor"
    "../../thirdparty/next/inc"
    "../../thirdparty/next/inc/math"
    "../../thirdparty/next/inc/core"
    "../../thirdparty/openal/include"
    "../../thirdparty/libogg/src"
    "../../thirdparty/libvorbis/src"
)

# find where OpenAL is installed on user's system
if(UNIX AND NOT APPLE)
    find_package(OpenAL REQUIRED)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        ${OPENAL_INCLUDE_DIR}
    )

    # This path is only needed on the BSDs
    if(NOT LINUX)
        set(${PROJECT_NAME}_incPaths
            ${${PROJECT_NAME}_incPaths}
            "/usr/local/include"
        )
    endif()
endif()

file(GLOB MOC_HEADERS
    "includes/converters/*.h"
)

QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

# Dynamic Library
if (desktop)
    add_library(${PROJECT_NAME}-editor SHARED
        ${${PROJECT_NAME}_srcFiles}
        ${MOC_SRCS}
        "src/converters/audioconverter.cpp"
    )

    if (WIN32)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/openal/windows/x64/OpenAL32.lib
        )
    endif ()

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        next-editor
        engine-editor
        ogg-editor
        vorbis-editor
        vorbisfile-editor
        Qt5::Core
        Qt5::Gui
        Qt5::Multimedia
    )
    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
        MEDIA_LIBRARY
    )

    if(UNIX AND NOT APPLE)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE ${OPENAL_LIBRARY})
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            INSTALL_RPATH "$ORIGIN/../../lib"
        )

        # Solve build error using Clang on BSDs
        if(NOT LINUX)
            target_compile_options(${PROJECT_NAME}-editor PRIVATE -fPIC)
        endif()
    endif()

    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${PLUGINS_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${PLUGINS_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}-editor
            DESTINATION "${PLUGINS_PATH}"
    )

    install(FILES "includes/${PROJECT_NAME}.h"
            DESTINATION "${INC_PATH}/modules"
    )

    install(FILES "includes/components/audiosource.h"
            DESTINATION "${INC_PATH}/engine"
    )
    
    install(FILES "includes/resources/audioclip.h"
            DESTINATION "${INC_PATH}/engine"
    )

    if (WIN32)
        install(FILES "../../thirdparty/openal/windows/bin/OpenAL32.dll" DESTINATION "${BIN_PATH}")
    endif()
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_link_libraries(${PROJECT_NAME} PRIVATE
    next
    engine
    ogg
    vorbis
)

if (NOT desktop)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        THUNDER_MOBILE
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME}-editor PRIVATE ${OPENAL_LIBRARY})

    # Solve build error using Clang on BSDs
    if(NOT LINUX)
        target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
    endif()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
)

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${STATIC_PATH}"
)
