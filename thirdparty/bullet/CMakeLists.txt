cmake_minimum_required(VERSION 3.10)

project(bullet3)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/LinearMath/*.cpp"
    "src/BulletCollision/CollisionDispatch/*.cpp"
    "src/BulletCollision/CollisionShapes/*.cpp"
    "src/BulletCollision/BroadphaseCollision/*.cpp"
    "src/BulletCollision/NarrowPhaseCollision/*.cpp"
    "src/BulletDynamics/Dynamics/*.cpp"
    "src/BulletDynamics/ConstraintSolver/*.cpp"
    "src/BulletDynamics/Character/*.cpp"
    "src/LinearMath/*.h"
    "src/BulletCollision/CollisionDispatch/*.h"
    "src/BulletCollision/CollisionShapes/*.h"
    "src/BulletCollision/BroadphaseCollision/*.h"
    "src/BulletCollision/NarrowPhaseCollision/*.h"
    "src/BulletDynamics/Dynamics/*.h"
    "src/BulletDynamics/ConstraintSolver/*.h"
    "src/BulletDynamics/Character/*.h"
    "src/*.h"
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

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_compile_definitions(${PROJECT_NAME} PRIVATE BULLET_EXPORT)
target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_RPATH TRUE
        VERSION ${PROJECT_VERSION}
    )
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-argument-outside-range")
endif()

# Android-specific configurations
if(ANDROID)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        ANDROID_NDK_PLATFORM ${ANDROID}
        ANDROID_NDK_ABI ${CMAKE_ANDROID_ARCH_ABI}
        ANDROID_STL ${ANDROID_STL}
    )
endif()

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()

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
