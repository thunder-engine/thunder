cmake_minimum_required(VERSION 3.10)

project(angelscript)

file(GLOB ${PROJECT_NAME}_srcFiles
    "source/*.cpp"
    "source/*.h"
)
set(${PROJECT_NAME}_incPaths "include")

if (MSVC AND CMAKE_CL_64)
    enable_language(ASM_MASM)
    if(CMAKE_ASM_MASM_COMPILER_WORKS)
        list(APPEND ${PROJECT_NAME}_srcFiles "source/as_callfunc_x64_msvc_asm.asm")
    endif()
endif ()

# Dynamic Library
if(desktop)
    add_library(${PROJECT_NAME}-editor SHARED ${${PROJECT_NAME}_srcFiles})

    target_compile_definitions(${PROJECT_NAME}-editor PRIVATE
        ANGELSCRIPT_EXPORT
        #AS_DEBUG
    )
    target_include_directories(${PROJECT_NAME}-editor PRIVATE ${${PROJECT_NAME}_incPaths})
    target_compile_features(${PROJECT_NAME}-editor PRIVATE cxx_std_14)

    set_target_properties(${PROJECT_NAME}-editor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../../${LIB_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../../${LIB_PATH}"
    )

    if(APPLE)
        set_target_properties(${PROJECT_NAME}-editor PROPERTIES
            MACOSX_RPATH TRUE
            LINK_FLAGS "-undefined dynamic_lookup"
            VERSION ${PROJECT_VERSION}
        )
    endif()

    install(TARGETS ${PROJECT_NAME}-editor
            LIBRARY DESTINATION ${LIB_PATH}
            ARCHIVE DESTINATION ${LIB_PATH}
            RUNTIME DESTINATION ${LIB_PATH}
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ANGELSCRIPT_EXPORT
    AS_NO_COMPILER
)
target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_RPATH TRUE
        VERSION ${PROJECT_VERSION}
    )
endif()

# Android-specific configurations
if(ANDROID)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        ANDROID_NDK_PLATFORM ${ANDROID}
        ANDROID_NDK_ABI ${CMAKE_ANDROID_ARCH_ABI}
        ANDROID_STL ${ANDROID_STL}
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
)

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${STATIC_PATH}
        ARCHIVE DESTINATION ${STATIC_PATH}
)
