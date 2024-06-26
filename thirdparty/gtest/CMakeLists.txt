project(GTest)

set(SOURCES
    src/gtest-all.cc
    src/gtest_main.cc)

add_library(GTest STATIC ${SOURCES})

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

target_include_directories(GTest PUBLIC include)
target_include_directories(GTest PRIVATE ${incPaths})
target_compile_definitions(GTest PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(GTest PRIVATE -fPIC)
endif()
