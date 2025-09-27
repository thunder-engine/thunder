project(GTest)

set(SOURCES
    src/gtest-all.cc
    src/gtest_main.cc)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

set(incPaths
    .
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(incPaths
        ${incPaths}
        "/usr/local/include"
    )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE ${incPaths})
target_compile_definitions(${PROJECT_NAME} PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "thirdparty")

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()
