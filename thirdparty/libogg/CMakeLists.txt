cmake_minimum_required(VERSION 3.10)

project(ogg)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.c"
)

set(${PROJECT_NAME}_incPaths
    "src"
    "src/ogg"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
)
endif()

# Dynamic Library
if(desktop)
    add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcFiles})
    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE LIBOGG_LIBRARY)
    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths})

    # Windows-specific configurations
    if(WIN32 AND NOT MINGW)
        target_sources(${PROJECT_NAME}-editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ogg.def)
    endif()

    # macOS-specific configurations
    if(APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            MACOSX_RPATH TRUE
            SUFFIX ".dylib"
        )
        target_compile_definitions(${PROJECT_NAME}-editor PRIVATE LIBOGG_LIBRARY)
    endif()

    # Solve build error using Clang on BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        target_compile_options(${PROJECT_NAME}-editor PRIVATE -fPIC)
    endif()

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${LIB_PATH}"
        FOLDER "thirdparty"
    )

    # Install
    install(TARGETS ${PROJECT_NAME}-editor
        LIBRARY DESTINATION ${LIB_PATH}
        ARCHIVE DESTINATION ${LIB_PATH}
        RUNTIME DESTINATION ${LIB_PATH}
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
    FOLDER "thirdparty"
)

# Install
install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${STATIC_PATH}
        ARCHIVE DESTINATION ${STATIC_PATH}
)
