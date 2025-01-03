cmake_minimum_required(VERSION 3.10)

project(glsl)

file(GLOB ${PROJECT_NAME}_srcFiles
    "glslang/GenericCodeGen/*.cpp"
    "glslang/MachineIndependent/*.cpp"
    "glslang/MachineIndependent/preprocessor/*.cpp"
    "SPIRV/*.cpp"
    "SPIRV/CInterface/*.cpp"
    "OGLCompilersDLL/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "./"
    "OGLCompilersDLL"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

# Static Library
if(desktop)
    add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})

    # Windows-specific configurations
    if(WIN32)
        list(APPEND ${PROJECT_NAME}_srcFiles "glslang/OSDependent/Windows/ossource.cpp")
    endif()

    # Unix-specific configurations
    if(UNIX)
        list(APPEND ${PROJECT_NAME}_srcFiles "glslang/OSDependent/Unix/ossource.cpp")
    endif()

    # Solve build error using Clang on BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
    endif()

    # Set the source files
    target_sources(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_srcFiles})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
    )

    # Install
    install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${STATIC_PATH}
        ARCHIVE DESTINATION ${STATIC_PATH}
    )
endif()
