set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(SRC_FILES
    builder.cpp    
    builder.h
    consolelog.h
    main.cpp
    
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/animconverter.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/assetmanager.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/effectconverter.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/fbxconverter.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/fontconverter.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/functionmodel.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/material/aconstvalue.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/material/acoordinates.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/material/agradient.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/material/amaterialparam.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/material/amathfunction.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/material/amathoperator.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/material/atexturesample.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/material/autils.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/materialconverter.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/prefabconverter.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/qbsbuilder.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/shaderbuilder.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/spirvconverter.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/textconverter.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include/textureconverter.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/animconverter.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/assetmanager.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/effectconverter.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/fbxconverter.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/fontconverter.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/functionmodel.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/materialconverter.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/prefabconverter.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/qbsbuilder.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/shaderbuilder.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/textconverter.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/src/textureconverter.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/pluginmanager/include/pluginmodel.h
    ${PROJECT_SOURCE_DIR}/develop/managers/pluginmanager/src/pluginmodel.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/projectmanager/include/projectmanager.h
    ${PROJECT_SOURCE_DIR}/develop/managers/projectmanager/include/projectmodel.h
    ${PROJECT_SOURCE_DIR}/develop/managers/projectmanager/src/projectmanager.cpp
    ${PROJECT_SOURCE_DIR}/develop/managers/projectmanager/src/projectmodel.cpp
    ${PROJECT_SOURCE_DIR}/develop/models/include/abstractschememodel.h
    ${PROJECT_SOURCE_DIR}/develop/models/include/baseobjectmodel.h
    ${PROJECT_SOURCE_DIR}/develop/models/src/baseobjectmodel.cpp
)
add_executable(builder ${SRC_FILES})
target_include_directories(builder PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR} # for config.h
    ${PROJECT_SOURCE_DIR}/develop/managers/assetmanager/include
    ${PROJECT_SOURCE_DIR}/develop/managers/projectmanager/include
    ${PROJECT_SOURCE_DIR}/develop/managers/pluginmanager/include
    ${PROJECT_SOURCE_DIR}/develop/models/include/
)

target_link_libraries(builder PRIVATE 
    engine-editor rendergl-editor 
    Qt5::Core Qt5::Gui
    fbx glsl spirvcross quazip-editor
    )
target_compile_definitions(builder PRIVATE NEXT_SHARED BUILDER)
target_compile_definitions(builder PUBLIC
    -DCOMPANY_NAME="FrostSpear"
    -DEDITOR_NAME="WorldEditor"
    -DPRODUCT_NAME="Thunder Engine"
    -DCOPYRIGHT_AUTHOR="Evgeniy Prikazchikov"
    -DSPONSORS="${SPONSORS}"
    -DCOPYRIGHT_YEAR="${COPYRIGHT_YEAR}"
    -DLEGAL="${LEGAL}"
    -DREVISION="${GIT_BRANCH}"
    -DBUILDER_NAME="Builder"
    -DSDK_VERSION="1.0"
#    LAUNCHER_VERSION = "1.0"
#    YEAR = 2017
    )

#Project {
#    id: builder
#
#    property stringList incPaths: [
#        "src",
#        "../",
#        "../common",
#        "../engine/includes",
#        "../engine/includes/resources",
#        "../develop/managers/codemanager/include",
#        "../develop/managers/projectmanager/include",
#        "../develop/managers/assetmanager/include",
#        "../develop/managers/pluginmanager/include",
#        "../develop/models/include",
#        "../modules/renders/rendergl/includes",
#        "../thirdparty/next/inc",
#        "../thirdparty/next/inc/math",
#        "../thirdparty/next/inc/core",
#        "../thirdparty/next/inc/anim",
#        "../thirdparty/physfs/inc",
#        "../thirdparty/glfw/inc",
#        "../thirdparty/fbx/inc",
#        "../thirdparty/zlib/src",
#        "../thirdparty/quazip/src",
#        "../thirdparty/glsl",
#        "../thirdparty/spirvcross/src"
#    ]
#
#    QtApplication {
#        name: builder.BUILDER_NAME
#        condition: builder.desktop
#        files: builder.srcFiles
#        Depends { name: "cpp" }
#        Depends { name: "quazip-editor" }
#
#        bundle.isBundle: false
#
#        cpp.includePaths: builder.incPaths
#        cpp.libraryPaths: [
#            "../thirdparty/fbx/lib"
#        ]
#
#        property string prefix: qbs.targetOS.contains("windows") ? "lib" : ""
#        cpp.dynamicLibraries: [
#            prefix + "fbxsdk"
#        ]
#        cpp.cxxLanguageVersion: "c++14"
#
#        Properties {
#            condition: qbs.targetOS.contains("linux")
#            cpp.rpaths: "$ORIGIN"
#        }
#
#        Properties {
#            condition: qbs.targetOS.contains("darwin")
#            cpp.sonamePrefix: "@rpath"
#            cpp.rpaths: "@executable_path/../Frameworks/"
#        }
#
#        Group {
#            name: "Install " + builder.BUILDER_NAME
#            fileTagsFilter: product.type
#            qbs.install: true
#            qbs.installDir: builder.BIN_PATH + "/" + builder.bundle
#            qbs.installPrefix: builder.PREFIX
#        }
#    }
#}
