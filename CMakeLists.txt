CMAKE_MINIMUM_REQUIRED(VERSION 3.11)

project(thunder)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(Qt5 COMPONENTS Core Gui Widgets Quick QuickWidgets)
find_package(Threads REQUIRED)
if(WIN32)
  add_definitions(-D_UNICODE -DUNICODE)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Output all built binaries (exe/dll) to a single directory
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(thirdparty)

add_subdirectory(engine)
add_subdirectory(modules)
add_subdirectory(worldeditor)

add_subdirectory(builder)