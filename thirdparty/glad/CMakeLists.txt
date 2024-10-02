cmake_minimum_required(VERSION 3.10)

project(glad)

file(GLOB ${PROJECT_NAME}_srcFiles "src/*.c")

set(${PROJECT_NAME}_incPaths "include")

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

# Static Library
if(desktop)
    add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
    target_compile_definitions(${PROJECT_NAME} PRIVATE)
    target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

    # Solve build error using Clang on BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
    endif()

    set_target_properties(${PROJECT_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "../../${STATIC_PATH}"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "../../${STATIC_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${STATIC_PATH}
        ARCHIVE DESTINATION ${STATIC_PATH}
    )
    
endif()
