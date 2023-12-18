cmake_minimum_required(VERSION 3.10)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

project(worldEditor)

file(GLOB ${PROJECT_NAME}_srcFiles
    "src/*.cpp"
    "src/*.h"
    "src/main/*.cpp"
    "src/managers/assetimporter/*.cpp"
    "src/managers/feedmanager/*.cpp"
    "src/managers/plugindialog/*.cpp"
    "src/managers/projectmanager/*.cpp"
    "src/managers/toolwindowmanager/*.cpp"
    "src/managers/toolwindowmanager/private/*.cpp"
    "src/screens/baseobjectmodel/*.cpp"
    "src/screens/componentbrowser/*.cpp"
    "src/screens/consoleoutput/*.cpp"
    "src/screens/contentbrowser/*.cpp"
    "src/screens/objecthierarchy/*.cpp"
    "src/screens/propertyedit/*.cpp"
    "src/screens/propertyedit/custom/alignment/*.cpp"
    "src/screens/propertyedit/custom/array/*.cpp"
    "src/screens/propertyedit/custom/axises/*.cpp"
    "src/screens/propertyedit/custom/color/*.cpp"
    "src/screens/propertyedit/custom/component/*.cpp"
    "src/screens/propertyedit/custom/filepath/*.cpp"
    "src/screens/propertyedit/custom/locale/*.cpp"
    "src/screens/propertyedit/custom/nextenum/*.cpp"
    "src/screens/propertyedit/custom/objectselect/*.cpp"
    "src/screens/propertyedit/custom/vector4/*.cpp"
    "src/screens/propertyedit/editors/*.cpp"
    "src/screens/preview/*.cpp"
    "src/screens/scenecomposer/*.cpp"
    "src/screens/scenecomposer/tools/*.cpp"
    "res/icon.rc"
    "res/app-Info.plist"
)

set(${PROJECT_NAME}_incPaths
    "src"
    "../"
    "../engine/includes"
    "../engine/includes/resources"
    "../engine/includes/components"
    "../thirdparty/next/inc"
    "../thirdparty/next/inc/math"
    "../thirdparty/next/inc/core"
    "../thirdparty/next/inc/anim"
    "../modules/editor/grapheditor"
)

file(GLOB ${PROJECT_NAME}_ui
    "src/main/*.ui"
    "src/managers/assetimporter/*.ui"
    "src/managers/plugindialog/*.ui"
    "src/screens/componentbrowser/*.ui"
    "src/screens/consoleoutput/*.ui"
    "src/screens/contentbrowser/*.ui"
    "src/screens/objecthierarchy/*.ui"
    "src/screens/propertyedit/*.ui"
    "src/screens/propertyedit/custom/alignment/*.ui"
    "src/screens/propertyedit/custom/array/*.ui"
    "src/screens/propertyedit/custom/axises/*.ui"
    "src/screens/propertyedit/custom/color/*.ui"
    "src/screens/propertyedit/custom/component/*.ui"
    "src/screens/propertyedit/custom/filepath/*.ui"
    "src/screens/propertyedit/custom/locale/*.ui"
    "src/screens/propertyedit/custom/nextenum/*.ui"
    "src/screens/propertyedit/custom/objectselect/*.ui"
    "src/screens/propertyedit/custom/vector4/*.ui"
    "src/screens/propertyedit/editors/*.ui"
    "src/screens/preview/*.ui"
    "src/screens/scenecomposer/*.ui"
)

file(GLOB MOC_HEADERS
    "src/*.h"
    "src/main/*.h"
    "src/managers/assetimporter/*.h"
    "src/managers/feedmanager/*.h"
    "src/managers/plugindialog/*.h"
    "src/managers/projectmanager/*.h"
    "src/managers/toolwindowmanager/*.h"
    "src/managers/toolwindowmanager/private/*.h"
    "src/screens/baseobjectmodel/*.h"
    "src/screens/componentbrowser/*.h"
    "src/screens/consoleoutput/*.h"
    "src/screens/contentbrowser/*.h"
    "src/screens/objecthierarchy/*.h"
    "src/screens/propertyedit/*.h"
    "src/screens/propertyedit/custom/alignment/*.h"
    "src/screens/propertyedit/custom/array/*.h"
    "src/screens/propertyedit/custom/axises/*.h"
    "src/screens/propertyedit/custom/color/*.h"
    "src/screens/propertyedit/custom/component/*.h"
    "src/screens/propertyedit/custom/filepath/*.h"
    "src/screens/propertyedit/custom/locale/*.h"
    "src/screens/propertyedit/custom/nextenum/*.h"
    "src/screens/propertyedit/custom/objectselect/*.h"
    "src/screens/propertyedit/custom/vector4/*.h"
    "src/screens/propertyedit/editors/*.h"
    "src/screens/preview/*.h"
    "src/screens/scenecomposer/*.h"
    "src/screens/scenecomposer/tools/*.h"
)

set(RESOURCES
    res/WorldEditor.qrc
)

add_definitions (
    -DCOMPANY_NAME="${COMPANY_NAME}"
    -DPRODUCT_NAME="${PRODUCT_NAME}"
    -DEDITOR_NAME="${EDITOR_NAME}"
    -DBUILDER_NAME="${BUILDER_NAME}"
    -DSDK_VERSION="${SDK_VERSION}"
    -DCOPYRIGHT_YEAR=${COPYRIGHT_YEAR}
    -DCOPYRIGHT_AUTHOR="${COPYRIGHT_AUTHOR}"
    -DREVISION="${REVISION}"
    -DLEGAL="${LEGAL}"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${PROJECT_NAME}_srcFiles})

if (desktop)
    add_executable(${EDITOR_NAME}
        ${${PROJECT_NAME}_srcFiles}
        ${MOC_HEADERS}
        ${RESOURCES}
        ${${PROJECT_NAME}_ui}
    )

    target_link_libraries(${EDITOR_NAME} PRIVATE
        assimp
        zlib-editor
        next-editor
        vorbis-editor
        ogg-editor
        engine-editor
        graph-editor
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Multimedia
        Qt5::QuickWidgets
        Qt5::Xml
    )

    if(WIN32)
        set_property(TARGET ${EDITOR_NAME} PROPERTY WIN32_EXECUTABLE true)
    endif()

    target_compile_definitions(${EDITOR_NAME} PRIVATE
        SHARED_DEFINE
    )

    target_include_directories(${EDITOR_NAME} PRIVATE ${${PROJECT_NAME}_incPaths} ${CMAKE_CURRENT_BINARY_DIR})

    set_target_properties(${EDITOR_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "../${BIN_PATH}"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "../${BIN_PATH}"
    )

    install(TARGETS ${EDITOR_NAME}
            DESTINATION "${BIN_PATH}"
    )

    if(APPLE)
        install(FILES
                "res/icons/thunder.icns"
                DESTINATION "${BIN_PATH}/../Resources"
        )
    endif()
endif()
