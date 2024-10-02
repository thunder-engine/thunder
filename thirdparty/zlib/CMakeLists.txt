cmake_minimum_required(VERSION 3.10)

project(zlib)

# zlib Library
file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.c"
)

set(${PROJECT_NAME}_incPaths
    "src"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

# Create zlib library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_include_directories(zlib PRIVATE ${${PROJECT_NAME}_incPaths})
target_compile_definitions(zlib PRIVATE ZLIB_LIBRARY)

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
)

# Install zlib library
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${STATIC_PATH}
    ARCHIVE DESTINATION ${STATIC_PATH}
)

# zlib Dynamic Library (Optional)
if(desktop)
    add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcFiles})
    target_include_directories(${PROJECT_NAME}-editor PRIVATE "src")
    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE ZLIB_DLL ZLIB_LIBRARY)

    # Additional properties for macOS
    if (APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES INSTALL_RPATH "@executable_path")
    endif()

    # Solve build error using Clang on BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        target_compile_options(${PROJECT_NAME}-editor PRIVATE -fPIC)
    endif()

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${LIB_PATH}"
    )

    # Install zlib Dynamic Library
    install(TARGETS ${PROJECT_NAME}-editor
            LIBRARY DESTINATION ${LIB_PATH}
            ARCHIVE DESTINATION ${LIB_PATH}
            RUNTIME DESTINATION ${LIB_PATH}
    )
endif()
