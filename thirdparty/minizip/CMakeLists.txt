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
	compat/ioapi.c
    compat/unzip.c
    compat/zip.c
    mz_zip.c
    mz_os.c
    mz_strm.c
    mz_strm_buf.c
    mz_strm_mem.c
    mz_crypt.c
)

if(WIN32)
    set(${PROJECT_NAME}_srcFiles
        ${${PROJECT_NAME}_srcFiles}
		mz_os_win32.c
        mz_strm_os_win32.c
    )
else()
    set(${PROJECT_NAME}_srcFiles
        ${${PROJECT_NAME}_srcFiles}
		mz_os_posix.c
        mz_strm_os_posix.c
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