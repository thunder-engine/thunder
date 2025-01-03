cmake_minimum_required(VERSION 3.10)

project(modules)

if(desktop)
    add_subdirectory(editor)
endif()

add_subdirectory(media)
add_subdirectory(physics/bullet)
add_subdirectory(renders)
add_subdirectory(uikit)
add_subdirectory(vms/angel)
