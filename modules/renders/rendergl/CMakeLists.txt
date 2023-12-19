cmake_minimum_required(VERSION 3.10)

project(rendergl)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/components/*.cpp"
    "src/resources/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "includes"
    "../../../common"
    "../../../engine/includes/resources"
    "../../../engine/includes"
    "../../../thirdparty/next/inc"
    "../../../thirdparty/next/inc/math"
    "../../../thirdparty/next/inc/core"
    "../../../thirdparty/glfw/include"
    "../../../thirdparty/glfm/include"
    "../../../thirdparty/glew/include"
    "../../../thirdparty/glad/include"
)

file(GLOB MOC_HEADERS
    "*.h"
    "includes/editor/*.h"
)

QT5_WRAP_CPP(MOC_SRCS ${MOC_HEADERS})

# Dynamic Library
if (desktop)
    add_library(${PROJECT_NAME}-editor SHARED
        ${${PROJECT_NAME}_srcFiles}
        ${MOC_SRCS}
        "src/editor/rhiwrapper.cpp"
    )

    set(${PROJECT_NAME}_libs
        next-editor
        engine-editor
        glfw-editor
        glad
        Qt5::Core
        Qt5::Gui
    )

    if(WIN32)
        list(APPEND ${PROJECT_NAME}_libs
            opengl32
            glu32
        )
    endif ()

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_libs})

    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
    )

    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../../${PLUGINS_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../../${PLUGINS_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}-editor
        DESTINATION "${PLUGINS_PATH}"
    )

    install(FILES "includes/${PROJECT_NAME}.h"
        DESTINATION "${INC_PATH}/modules"
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})

target_link_libraries(${PROJECT_NAME} PRIVATE
    next
    engine
    glfw
    glad
)

target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

if (NOT desktop)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        THUNDER_MOBILE
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../../${STATIC_PATH}"
)

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${STATIC_PATH}"
)
