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
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

if(desktop)
    # This path is only needed on the BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        set(${PROJECT_NAME}_incPaths
            ${${PROJECT_NAME}_incPaths}
            ${Qt5Gui_INCLUDE_DIRS}
            ${Qt5Svg_INCLUDE_DIRS}
            ${Qt5Widgets_INCLUDE_DIRS}
            ${Qt5Xml_INCLUDE_DIRS}
            "/usr/local/include"
            "includes/editor"
            "../thirdparty/assimp/include"
        )
    endif()

    file(GLOB MOC_HEADERS
        "includes/editor/*.h"
        "includes/editor/viewport/*.h"
        "includes/editor/converters/*.h"
    )

    file(GLOB EDITOR_SOURCES
        "src/editor/*.cpp"
    )

    QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

    # Dynamic Library
    add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcFiles} ${MOC_SRCS} ${EDITOR_SOURCES})

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        assimp
        next-editor
        glfw-editor
        zlib-editor
        physfs-editor
        freetype-editor
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Svg
        Qt5::Xml
    )

    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
        ENGINE_LIBRARY
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

    file(GLOB ${PROJECT_NAME}_editorFiles
        "src/editor/*.cpp"
        "src/editor/converters/*.cpp"
        "src/editor/viewport/*.cpp"
    )

    target_sources(${PROJECT_NAME}-editor PRIVATE
        "src/adapters/platformadaptor.cpp"
        "src/adapters/desktopadaptor.cpp"
        ${${PROJECT_NAME}_editorFiles}
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

    install(DIRECTORY includes/editor/
        DESTINATION "${INC_PATH}/editor"
        FILES_MATCHING PATTERN "*.h"
        PATTERN "converters" EXCLUDE
    )

endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_compile_definitions(${PROJECT_NAME} PRIVATE
    NEXT_LIBRARY
)

target_sources(${PROJECT_NAME} PRIVATE
    "src/adapters/platformadaptor.cpp"
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
