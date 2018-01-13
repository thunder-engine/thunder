# Variables
COMPANY_NAME = "FrostSpear"
EDITOR_NAME = "WorldEditor"
BUILDER_NAME = "Builder"
SDK_VERSION = "1.0"
LAUNCHER_VERSION = "1.0"
YEAR = 2017

DEFINES +=  COMPANY_NAME='"\\\"$${COMPANY_NAME}\\\""' \
            EDITOR_NAME='"\\\"$${EDITOR_NAME}\\\""' \
            BUILDER_NAME='"\\\"$${BUILDER_NAME}\\\""' \
            SDK_VERSION='"\\\"$${SDK_VERSION}\\\""' \
            LAUNCHER_VERSION='"\\\"$${LAUNCHER_VERSION}\\\""' \
            YEAR=$${YEAR}

SPEC = vc2013
SPECVER = 120

BUILD_TYPE = ""
BUILD_DIR = "release"
LIB_PREFIX = ""
CRT = ""
CONFIG(debug, debug|release) {
    BUILD_TYPE = "Debug"
    BUILD_DIR = "debug"
    LIB_PREFIX = "d"
    CRT = "/Debug_NonRedist"
}

isEmpty(PREFIX) {
    PREFIX = _PACKAGE
}

EXTERNAL = ../external

# Paths
QBS_ROOT = D:/Environment/qbs-windows-x86_64-1.9.1

NEXT_INCLUDE = $${EXTERNAL}/next/inc
NEXT_SHARED = $${EXTERNAL}-target/libs/next/shared/windows_x32/$${BUILD_DIR}
NEXT_STATIC = $${EXTERNAL}-target/libs/next/static/windows_x32/release

ZLIB_INCLUDE = $${EXTERNAL}/zlib/inc
ZLIB_SHARED = $${EXTERNAL}-target/libs/zlib/shared/windows_x32/$${BUILD_DIR}
ZLIB_STATIC = $${EXTERNAL}-target/libs/zlib/static/windows_x32/release

PHYSFS_INCLUDE = $${EXTERNAL}/physfs/inc
PHYSFS_SHARED = $${EXTERNAL}-target/libs/physfs/shared/windows_x32/$${BUILD_DIR}
PHYSFS_STATIC = $${EXTERNAL}-target/libs/physfs/static/windows_x32/release

GLFW_INCLUDE = $${EXTERNAL}/glfw/inc
GLFW_LIBRARY = $${GLFW_INCLUDE}/../lib/lib-$${SPEC}
GLFW_BINARY = $${GLFW_LIBRARY}

GLEW_INCLUDE = $${EXTERNAL}/nvidia/inc
GLEW_LIBRARY = $${GLEW_INCLUDE}/../lib
GLEW_BINARY = $${GLEW_INCLUDE}/../bin

QUAZIP_INCLUDE = $${EXTERNAL}/quazip/inc
QUAZIP_SHARED = $${QUAZIP_INCLUDE}/../lib

FBX_INCLUDE = $${EXTERNAL}/fbx/inc
FBX_LIBRARY = $${FBX_INCLUDE}/../lib
FBX_BINARY = $${FBX_INCLUDE}/../bin

SSL_BINARY = ../external/openssl/bin/win32
CRT_BINARY = $$(VS120COMNTOOLS)../../VC/redist$${CRT}/$$QMAKE_TARGET.arch/Microsoft.VC$${SPECVER}.$${BUILD_TYPE}CRT

RESOURCE_ROOT = $$PWD/worldeditor/bin

win32: {
    DEFINES += PLATFORM_WINDOWS
    QMAKE_CXXFLAGS += -Zc:wchar_t -Zi -D_WIN32_WINNT=0x0601 -DWINVER=0x0601 -D_CRT_SECURE_NO_WARNINGS
    PLATFORM = "windows/$${QMAKE_TARGET.arch}"
}

LAUNCHER_PATH = $${PREFIX}/launcher
SDK_PATH = $${PREFIX}/sdk/$${SDK_VERSION}
