cmake_minimum_required(VERSION 3.10)

project(physfs)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.c"
    "src/archivers/*.c"
)

if (WIN32)
    set(${PROJECT_NAME}_platform
        "src/platform/win32.c"
    ) 
else ()
    set(${PROJECT_NAME}_platform
        "src/platform/unix.c"
        "src/platform/posix.c"
    )
endif()

set(${PROJECT_NAME}_incPaths
    "src"
    "../zlib/src"
)

# Dynamic Library
if(desktop)
    add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcFiles} ${${PROJECT_NAME}_platform})
    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        PHYSFS_SUPPORTS_ZIP
        PHYSFS_NO_CDROM_SUPPORT)
    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths})
    target_link_libraries(${PROJECT_NAME}-editor PRIVATE zlib-editor)

    # Windows-specific configurations
    if(WIN32)
        target_link_libraries(${PROJECT_NAME}-editor PRIVATE Advapi32)
    endif()

    # macOS-specific configurations
    if(APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            MACOSX_RPATH TRUE
            SUFFIX ".dylib"
        )
        target_compile_definitions(${PROJECT_NAME}-editor PRIVATE PHYSFS_DARWIN)
    endif()

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${LIB_PATH}"
    )

    # Install
    install(TARGETS ${PROJECT_NAME}-editor
            LIBRARY DESTINATION ${LIB_PATH}
            ARCHIVE DESTINATION ${LIB_PATH}
            RUNTIME DESTINATION ${LIB_PATH}
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles} ${${PROJECT_NAME}_platform})
target_compile_definitions(${PROJECT_NAME} PRIVATE PHYSFS_SUPPORTS_ZIP PHYSFS_NO_CDROM_SUPPORT)
target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

# macOS-specific configurations
if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PHYSFS_DARWIN)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
)

# Install
install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${STATIC_PATH}
        ARCHIVE DESTINATION ${STATIC_PATH}
)
