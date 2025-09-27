cmake_minimum_required(VERSION 3.10)

project(minizip)


set(${PROJECT_NAME}_incPaths
    "./"
	"../zlib/src"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

set(${PROJECT_NAME}_srcFiles
	ioapi.c
	miniunz.c
	mztools.c
	unzip.c
	zip.c
)

if(WIN32)
    set(${PROJECT_NAME}_srcFiles
        ${${PROJECT_NAME}_srcFiles}
		iowin32.c
    )
endif()

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "thirdparty")

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()