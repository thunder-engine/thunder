cmake_minimum_required(VERSION 3.10)

project(basisu)

set(${PROJECT_NAME}_incPaths
    "encoder"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

set(${PROJECT_NAME}_srcFiles
    encoder/3rdparty/android_astc_decomp.cpp
    encoder/3rdparty/tinyexr.cpp
    encoder/basisu_astc_hdr_6x6_enc.cpp
    encoder/basisu_astc_hdr_common.cpp
    encoder/basisu_backend.cpp
    encoder/basisu_basis_file.cpp
    encoder/basisu_bc7enc.cpp
    encoder/basisu_comp.cpp
    encoder/basisu_enc.cpp
    encoder/basisu_etc.cpp
    encoder/basisu_frontend.cpp
    encoder/basisu_gpu_texture.cpp
    encoder/basisu_kernels_sse.cpp
    encoder/basisu_opencl.cpp
    encoder/basisu_pvrtc1_4.cpp
    encoder/basisu_resample_filters.cpp
    encoder/basisu_resampler.cpp
    encoder/basisu_ssim.cpp
    encoder/basisu_uastc_enc.cpp
    encoder/basisu_uastc_hdr_4x4_enc.cpp
    encoder/jpgd.cpp
    encoder/pvpngreader.cpp
    transcoder/basisu_transcoder.cpp
    zstd/zstd.c
)

# Static Library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles})
target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "thirdparty")

# Solve build error using Clang on BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()
