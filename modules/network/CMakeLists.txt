cmake_minimum_required(VERSION 3.10)

project(network)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/objects/*.cpp"
    "src/utils/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "includes"
    "includes/objects"
    "includes/utils"
    "../../common"
    "../../engine/includes"
    "../../engine/includes/resources"
    "../../engine/includes/components"
    "../../engine/includes/editor"
    "../../thirdparty/next/inc"
    "../../thirdparty/next/inc/math"
    "../../thirdparty/next/inc/core"
    "../../thirdparty/ssl/include"
)

if(UNIX AND NOT APPLE)
    # This path is only needed on the BSDs
    if(NOT LINUX)
        set(${PROJECT_NAME}_incPaths
            ${${PROJECT_NAME}_incPaths}
            "/usr/local/include"
        )
    endif()
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

# Dynamic Library
if(desktop)
    file(GLOB MOC_HEADERS
        "includes/converters/*.h"
    )

    add_library(${PROJECT_NAME}-editor SHARED
        ${${PROJECT_NAME}_srcFiles}
        ${MOC_SRCS}
    )

    if(WIN32)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE 
            Ws2_32
            Crypt32
            Advapi32
            User32
        )
        target_link_directories(${PROJECT_NAME}-editor
            PUBLIC "../../thirdparty/ssl/lib"
        )
    endif()

    target_link_libraries(${PROJECT_NAME}-editor PRIVATE
        next-editor
        engine-editor
        ssl
        crypto
    )
    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        SHARED_DEFINE
        NETWORK_LIBRARY
    )

    if(UNIX AND NOT APPLE)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE ${NETWORK_LIBRARY})
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
        FOLDER "modules"
    )

    install(TARGETS ${PROJECT_NAME}-editor
            DESTINATION "${PLUGINS_PATH}"
    )

endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_link_libraries(${PROJECT_NAME} PRIVATE
    next
    engine
)

if(NOT desktop)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        THUNDER_MOBILE
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${NETWORK_LIBRARY})

    # Solve build error using Clang on BSDs
    if(NOT LINUX)
        target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
    endif()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
    FOLDER "modules"
)

install(TARGETS ${PROJECT_NAME}
    DESTINATION "${STATIC_PATH}"
)
