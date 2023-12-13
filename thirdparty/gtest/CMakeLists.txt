project(GTest)

set(SOURCES
    src/gtest-all.cc
    src/gtest_main.cc)

add_library(GTest STATIC ${SOURCES})

target_include_directories(GTest PUBLIC include)
target_include_directories(GTest PRIVATE .)
target_compile_definitions(GTest PUBLIC _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
