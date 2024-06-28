cmake_minimum_required(VERSION 3.10)

project(glfm)

set(srcFiles
    "src/glfm_platform.h"
    "include/glfm.h"
)

set(incPaths "include")

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

# Static Library
if(NOT desktop)
    add_library(glfm STATIC ${srcFiles})
    target_compile_definitions(glfm PRIVATE)
    target_include_directories(glfm PRIVATE ${incPaths})
    target_compile_features(glfm PRIVATE c_std_11)

    # Solve build error using Clang on BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
    endif()

    # Android-specific configurations
    if(ANDROID)
        set(nativePath ${CMAKE_ANDROID_NDK}/sources/android/native_app_glue)
        list(APPEND srcFiles
            "src/glfm_platform_android.c"
            "${nativePath}/android_native_app_glue.c"
            "${nativePath}/android_native_app_glue.h"
        )
        target_include_directories(glfm PRIVATE ${nativePath})
        set_target_properties(glfm PROPERTIES
            COMPILE_OPTIONS "-std=c11"
        )
    endif()

    # iOS and tvOS-specific configurations
    if(IOS OR TVOS)
        list(APPEND srcFiles "src/glfm_platform_ios.m")
    endif()

    # Set the source files
    target_sources(glfm PRIVATE ${srcFiles})

    # Install
    install(TARGETS glfm
            LIBRARY DESTINATION ${SDK_PATH}/${CMAKE_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}/static
            ARCHIVE DESTINATION ${SDK_PATH}/${CMAKE_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}/static
    )
endif()
