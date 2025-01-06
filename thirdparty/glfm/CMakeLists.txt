cmake_minimum_required(VERSION 3.10)

project(glfm)

set(${PROJECT_NAME}_srcFiles
    "src/glfm_internal.h"
    "include/glfm.h"
)

set(${PROJECT_NAME}_incPaths "include")

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

# Static Library
if(NOT desktop)
    add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
    target_compile_definitions(${PROJECT_NAME} PRIVATE)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})
    target_compile_features(${PROJECT_NAME} PRIVATE c_std_11)

    # Solve build error using Clang on BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
        set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
    endif()

    # Android-specific configurations
    if(ANDROID)
        set(nativePath ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue)
        list(APPEND srcFiles
            "src/glfm_android.c"
            "${nativePath}/android_native_app_glue.c"
            "${nativePath}/android_native_app_glue.h"
        )
        target_include_directories(${PROJECT_NAME} PRIVATE ${nativePath})
        set_target_properties(${PROJECT_NAME} PROPERTIES
            COMPILE_OPTIONS "-std=c11"
        )
    endif()

    if(EMSCRIPTEN)
        list(APPEND srcFiles "src/glfm_emscripten.c")
    endif()

    # iOS and tvOS-specific configurations
    if(IOS OR TVOS)
        list(APPEND srcFiles "src/glfm_apple.m")
    endif()

    # Set the source files
    target_sources(${PROJECT_NAME} PRIVATE ${srcFiles})

    # Install
    install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION "${STATIC_PATH}"
        ARCHIVE DESTINATION "${STATIC_PATH}"
    )
endif()
