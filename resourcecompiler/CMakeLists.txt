cmake_minimum_required(VERSION 3.10)

project(trc)

if(desktop)
    add_executable(${PROJECT_NAME} main.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE pugixml)
    target_include_directories(${PROJECT_NAME} PRIVATE
        "../thirdparty/pugixml/src"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "../${BIN_PATH}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "../${BIN_PATH}"
    )

    install(TARGETS ${PROJECT_NAME}
        DESTINATION "${TOOLS_PATH}"
    )
endif()
