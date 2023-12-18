cmake_minimum_required(VERSION 3.10)

project(thunder)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install_target)

set(COMPANY_NAME "FrostSpear")
set(PRODUCT_NAME "Thunder Engine")
set(EDITOR_NAME "WorldEditor")
set(BUILDER_NAME "Builder")
set(COPYRIGHT_AUTHOR "Evgeniy Prikazchikov")

set(ANDROID_VERSION "android-21")

string(TOLOWER ${CMAKE_SYSTEM_NAME} OS_NAME)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
    set(PLATFORM "${OS_NAME}/x86_64")
else()
    set(PLATFORM "${OS_NAME}/x86")
endif()

set(ANDROID_STL "c++_shared")

set(osxVersion "10.12")
set(iosVersion "10.0")
set(tvosVersion "10.0")

set(CMAKE_CXX_STANDARD 14)
set(standardLibrary "libc++")

set(desktop NOT ("android" IN_LIST CMAKE_SYSTEM_NAME OR "ios" IN_LIST CMAKE_SYSTEM_NAME OR "tvos" IN_LIST CMAKE_SYSTEM_NAME))

if (APPLE)
    set(bundle "${EDITOR_NAME}.app/Contents/MacOS/")
else()
    set(bundle "")
endif()

# Configure Git information
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE REVISION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

file(READ ${CMAKE_SOURCE_DIR}/legal LEGAL)
file(READ ${CMAKE_SOURCE_DIR}/version.cfg SDK_VERSION)

string(TIMESTAMP YEAR "%Y" UTC)

# Set project properties
set(COPYRIGHT_YEAR ${YEAR})
set(RESOURCE_ROOT "../worldeditor/bin/")

set(PREFIX "")
set(LAUNCHER_PATH "launcher")
set(SDK_PATH "sdk/${SDK_VERSION}")
set(PLATFORM_PATH "${SDK_PATH}/${PLATFORM}")
set(BIN_PATH "${PLATFORM_PATH}/bin")

if (UNIX)
    set(LIB_PATH "${PLATFORM_PATH}/lib")
else ()
    set(LIB_PATH ${BIN_PATH})
endif()

set(STATIC_PATH "${PLATFORM_PATH}/static")
set(INC_PATH "${SDK_PATH}/include")
set(TOOLS_PATH "${SDK_PATH}/tools")
set(PLUGINS_PATH "${BIN_PATH}/plugins")

find_package(Qt5 COMPONENTS
    Core
    Gui
    Widgets
    QuickWidgets
    Xml
    Multimedia
    Gamepad
    Svg
)

set(QT_BIN_DIR "$ENV{QT5_DIR}/bin")
set(QT_PLUGINS_DIR "$ENV{QT5_DIR}/plugins")
set(QT_QML_DIR "$ENV{QT5_DIR}/qml")

# Add subdirectories
add_subdirectory(thirdparty)
add_subdirectory(engine)
add_subdirectory(modules)
add_subdirectory(worldeditor)
add_subdirectory(builder)
add_subdirectory(tests)
add_subdirectory(build)
