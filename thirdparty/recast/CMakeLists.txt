cmake_minimum_required(VERSION 3.10)

project(recast)

file(GLOB ${PROJECT_NAME}_srcFiles
    "Detour/Source/*.cpp"
    "DetourCrowd/Source/*.cpp"
    "DetourTileCache/Source/*.cpp"
    "Recast/Source/*.cpp"
)

set(${PROJECT_NAME}_incPaths
    "DebugUtils/Include"
    "Detour/Include"
    "DetourCrowd/Include"
    "DetourTileCache/Include"
    "Recast/Include"
)

if(desktop)
    file(GLOB ${PROJECT_NAME}_debugFiles
        "DebugUtils/Source/*.cpp"
    )
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${${PROJECT_NAME}_srcFiles}
    ${${PROJECT_NAME}_debugFiles}
)

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_srcFiles}
    ${${PROJECT_NAME}_debugFiles}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if(ANDROID)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        ANDROID_NDK_PLATFORM ${ANDROID}
        ANDROID_NDK_ABI ${CMAKE_ANDROID_ARCH_ABI}
        ANDROID_STL ${ANDROID_STL}
    )
endif()

if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
    FOLDER "thirdparty"
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${STATIC_PATH}
    ARCHIVE DESTINATION ${STATIC_PATH}
)
