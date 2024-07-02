cmake_minimum_required(VERSION 3.10)

project(assimp)

# needed for config.h.in to work properly
set(CMAKE_INCLUDE_CURRENT_DIR ON)
configure_file(include/assimp/config.h.in include/assimp/config.h)

file(GLOB ${PROJECT_NAME}_srcFiles
    "code/AssetLib/*.cpp"
    "code/AssetLib/*.h"
    "code/AssetLib/3DS/*.cpp"
    "code/AssetLib/3DS/*.h"
    "code/AssetLib/3MF/*.cpp"
    "code/AssetLib/3MF/*.h"
    "code/AssetLib/AC/*.cpp"
    "code/AssetLib/AC/*.h"
    "code/AssetLib/AMF/*.cpp"
    "code/AssetLib/AMF/*.h"
    "code/AssetLib/ASE/*.cpp"
    "code/AssetLib/ASE/*.h"
    "code/AssetLib/Assbin/*.cpp"
    "code/AssetLib/Assbin/*.h"
    "code/AssetLib/Assjson/*.cpp"
    "code/AssetLib/Assjson/*.h"
    "code/AssetLib/Assxml/*.cpp"
    "code/AssetLib/Assxml/*.h"
    "code/AssetLib/B3D/*.cpp"
    "code/AssetLib/B3D/*.h"
    "code/AssetLib/Blender/*.cpp"
    "code/AssetLib/Blender/*.h"
    "code/AssetLib/BVH/*.cpp"
    "code/AssetLib/BVH/*.h"
    "code/AssetLib/C4D/*.cpp"
    "code/AssetLib/C4D/*.h"
    "code/AssetLib/COB/*.cpp"
    "code/AssetLib/COB/*.h"
    "code/AssetLib/Collada/*.cpp"
    "code/AssetLib/Collada/*.h"
    "code/AssetLib/CSM/*.cpp"
    "code/AssetLib/CSM/*.h"
    "code/AssetLib/DXF/*.cpp"
    "code/AssetLib/DXF/*.h"
    "code/AssetLib/FBX/*.cpp"
    "code/AssetLib/FBX/*.h"
    "code/AssetLib/glTF/*.cpp"
    "code/AssetLib/glTF/*.h"
    "code/AssetLib/glTF2/*.cpp"
    "code/AssetLib/glTF2/*.h"
    "code/AssetLib/HMP/*.cpp"
    "code/AssetLib/HMP/*.h"
    "code/AssetLib/IFC/*.cpp"
    "code/AssetLib/IFC/*.h"
    "code/AssetLib/IQM/*.cpp"
    "code/AssetLib/IQM/*.h"
    "code/AssetLib/Irr/*.cpp"
    "code/AssetLib/Irr/*.h"
    "code/AssetLib/LWO/*.cpp"
    "code/AssetLib/LWO/*.h"
    "code/AssetLib/LWS/*.cpp"
    "code/AssetLib/LWS/*.h"
    "code/AssetLib/M3D/*.cpp"
    "code/AssetLib/M3D/*.h"
    "code/AssetLib/MD2/*.cpp"
    "code/AssetLib/MD2/*.h"
    "code/AssetLib/MD3/*.cpp"
    "code/AssetLib/MD3/*.h"
    "code/AssetLib/MD4/*.cpp"
    "code/AssetLib/MD4/*.h"
    "code/AssetLib/MD5/*.cpp"
    "code/AssetLib/MD5/*.h"
    "code/AssetLib/MDC/*.cpp"
    "code/AssetLib/MDC/*.h"
    "code/AssetLib/MDL/*.cpp"
    "code/AssetLib/MDL/*.h"
    "code/AssetLib/MDL/HalfLife/*.cpp"
    "code/AssetLib/MDL/HalfLife/*.h"
    "code/AssetLib/MMD/*.cpp"
    "code/AssetLib/MMD/*.h"
    "code/AssetLib/MS3D/*.cpp"
    "code/AssetLib/MS3D/*.h"
    "code/AssetLib/NDO/*.cpp"
    "code/AssetLib/NDO/*.h"
    "code/AssetLib/NFF/*.cpp"
    "code/AssetLib/NFF/*.h"
    "code/AssetLib/Obj/*.cpp"
    "code/AssetLib/Obj/*.h"
    "code/AssetLib/OFF/*.cpp"
    "code/AssetLib/OFF/*.h"
    "code/AssetLib/Ogre/*.cpp"
    "code/AssetLib/Ogre/*.h"
    "code/AssetLib/OpenGEX/*.cpp"
    "code/AssetLib/OpenGEX/*.h"
    "code/AssetLib/Ply/*.cpp"
    "code/AssetLib/Ply/*.h"
    "code/AssetLib/Q3BSP/*.cpp"
    "code/AssetLib/Q3BSP/*.h"
    "code/AssetLib/Q3D/*.cpp"
    "code/AssetLib/Q3D/*.h"
    "code/AssetLib/Raw/*.cpp"
    "code/AssetLib/Raw/*.h"
    "code/AssetLib/SIB/*.cpp"
    "code/AssetLib/SIB/*.h"
    "code/AssetLib/SMD/*.cpp"
    "code/AssetLib/SMD/*.h"
    "code/AssetLib/Step/*.cpp"
    "code/AssetLib/Step/*.h"
    "code/AssetLib/STEPParser/*.cpp"
    "code/AssetLib/STEPParser/*.h"
    "code/AssetLib/STL/*.cpp"
    "code/AssetLib/STL/*.h"
    "code/AssetLib/Terragen/*.cpp"
    "code/AssetLib/Terragen/*.h"
    "code/AssetLib/Unreal/*.cpp"
    "code/AssetLib/Unreal/*.h"
    "code/AssetLib/X/*.cpp"
    "code/AssetLib/X/*.h"
    "code/AssetLib/X3D/*.cpp"
    "code/AssetLib/X3D/*.h"
    "code/AssetLib/XGL/*.cpp"
    "code/AssetLib/XGL/*.h"
    "code/CApi/*.cpp"
    "code/CApi/*.h"
    "code/Common/*.cpp"
    "code/Common/*.h"
    "code/Geometry/*.cpp"
    "code/Geometry/*.h"
    "code/Material/*.cpp"
    "code/Material/*.h"
    "code/Pbrt/*.cpp"
    "code/Pbrt/*.h"
    "code/PostProcessing/*.cpp"
    "code/PostProcessing/*.h"
)

file(GLOB ${PROJECT_NAME}_unzip
    "../unzip/*.c"
    "../unzip/*.h"
)

set(${PROJECT_NAME}_incPaths
    "./"
    "../"
    "include"
    "code"
    "../pugixml/src"
    "../poly2tri"
    "../zlib/src"
    "../rapidjson/include"
    "../utf8cpp/source"
    "../unzip/"
)

# This path is only needed on the BSDs
if(UNIX AND NOT APPLE AND NOT LINUX)
    set(${PROJECT_NAME}_incPaths
        ${${PROJECT_NAME}_incPaths}
        "/usr/local/include"
    )
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

# Static Library
if(desktop)
    add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_srcFiles} ${${PROJECT_NAME}_unzip})
    target_compile_definitions(assimp PRIVATE
        ASSIMP_USE_HUNTER
        ASSIMP_BUILD_NO_OWN_ZLIB
        ASSIMP_BUILD_NO_EXPORT
        ASSIMP_BUILD_NO_IFC_IMPORTER
        ASSIMP_BUILD_NO_OPENGEX_IMPORTER
        ASSIMP_BUILD_NO_C4D_IMPORTER
        ASSIMP_BUILD_NO_STEP_IMPORTER
        ASSIMP_BUILD_NO_OGRE_IMPORTER
        ASSIMP_BUILD_NO_IRR_IMPORTER
        ASSIMP_BUILD_NO_SIB_IMPORTER
        ASSIMP_BUILD_NO_COLLADA_IMPORTER
        ASSIMP_BUILD_NO_MMD_IMPORTER
        ASSIMP_BUILD_NO_XGL_IMPORTER
    )
    target_include_directories(${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_incPaths})
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_14)

    if(APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_RPATH TRUE
            VERSION ${PROJECT_VERSION}
        )
    endif()

    # Solve build error using Clang on BSDs
    if(UNIX AND NOT APPLE AND NOT LINUX)
        target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
    endif()

endif()
