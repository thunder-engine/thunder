include(../_Paths.pri)

QT += core gui opengl

TARGET = ../../$${SDK_PATH}/bin/$${PLATFORM}/WorldEditor
TEMPLATE = app

QMAKE_CXXFLAGS_WARN_ON -= -w34100

INCLUDEPATH += \
    ../worldeditor/src \
    ../common \
    ../engine/includes \
    ../develop/managers/codemanager/include \
    ../develop/managers/projectmanager/include \
    ../develop/managers/assetmanager/include \
    ../develop/models/include \
    ../modules/renders/rendergl/includes \
    ../$${NEXT_INCLUDE} \
    ../$${NEXT_INCLUDE}/math \
    ../$${NEXT_INCLUDE}/core \
    ../$${ZLIB_INCLUDE} \
    ../$${GLEW_INCLUDE} \
    ../$${PHYSFS_INCLUDE} \
    ../$${FBX_INCLUDE}

LIBS    += \
    -L../$${SDK_PATH}/bin/$${PLATFORM} \
    -L../$${NEXT_SHARED} \
    -L../$${ZLIB_SHARED} \
    -L../$${GLEW_LIBRARY} \
    -L../$${FBX_LIBRARY} \
    -lnext \
    -lengine$${LIB_PREFIX} \
    -lrendergl$${LIB_PREFIX} \
    -lAdvapi32 \
    -lWinmm \
    -lWldap32 \
    -lzlib \
    -lfbxsdk-2012.1 \
    -lglew32 \
    -lopengl32 \
    -lglu32

win32:RC_FILE = res/icon.rc

RESOURCES += \
    res/WorldEditor.qrc

bin.path = $$OUT_PWD/../$${SDK_PATH}/bin/$${PLATFORM}
bin.files = \
    ../$${NEXT_SHARED}/next.dll \
    ../$${ZLIB_SHARED}/zlib.dll \
    ../$${PHYSFS_SHARED}/physfs.dll \
    ../$${GLFW_BINARY}/glfw3.dll \
    ../$${GLEW_BINARY}/glew32.dll \
    ../$${FBX_BINARY}/fbxsdk-2012.1.dll

qbs.path = $$OUT_PWD/../$${SDK_PATH}/bin/tools/qbs
qbs.files = \
    $${QBS_ROOT}/*

win32: {
    bin.files += \
        $${CRT_BINARY}/msvcr$${SPECVER}$${LIB_PREFIX}.dll \
        $${CRT_BINARY}/msvcp$${SPECVER}$${LIB_PREFIX}.dll
}

qt.path = $$bin.path
qt.files = \
    $$[QT_INSTALL_BINS]/Qt5Core$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_BINS]/Qt5Gui$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_BINS]/Qt5Network$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_BINS]/Qt5OpenGL$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_BINS]/Qt5Quick$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_BINS]/Qt5Widgets$${LIB_PREFIX}.dll

platforms.path = $$bin.path/platforms
platforms.files = \
    $$[QT_INSTALL_PLUGINS]/platforms/qwindows$${LIB_PREFIX}.dll

imageformats.path = $$bin.path/imageformats
imageformats.files = \
    $$[QT_INSTALL_PLUGINS]/imageformats/qdds$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_PLUGINS]/imageformats/qico$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_PLUGINS]/imageformats/qjp2$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_PLUGINS]/imageformats/qjpeg$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_PLUGINS]/imageformats/qmng$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_PLUGINS]/imageformats/qsvg$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_PLUGINS]/imageformats/qtga$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_PLUGINS]/imageformats/qtiff$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_PLUGINS]/imageformats/qwbmp$${LIB_PREFIX}.dll \
    $$[QT_INSTALL_PLUGINS]/imageformats/qwebp$${LIB_PREFIX}.dll

resources.path = $$OUT_PWD/../$${SDK_PATH}/resources

shaders.path = $$resources.path/shaders
shaders.files = \
    $$RESOURCE_ROOT/shaders/BRDF.frag \
    $$RESOURCE_ROOT/shaders/BRDF.frag.set \
    $$RESOURCE_ROOT/shaders/Blur.frag \
    $$RESOURCE_ROOT/shaders/Blur.frag.set \
    $$RESOURCE_ROOT/shaders/PostEffect.frag \
    $$RESOURCE_ROOT/shaders/PostEffect.frag.set \
    $$RESOURCE_ROOT/shaders/DirectLight.frag \
    $$RESOURCE_ROOT/shaders/DirectLight.frag.set \
    $$RESOURCE_ROOT/shaders/Downsample.frag \
    $$RESOURCE_ROOT/shaders/Downsample.frag.set \
    $$RESOURCE_ROOT/shaders/Surface.frag \
    $$RESOURCE_ROOT/shaders/Surface.frag.set \
    $$RESOURCE_ROOT/shaders/BasePass.vert \
    $$RESOURCE_ROOT/shaders/BasePass.vert.set \
    $$RESOURCE_ROOT/shaders/Common.vert \
    $$RESOURCE_ROOT/shaders/Common.vert.set \
    $$RESOURCE_ROOT/shaders/SSAA.frag \
    $$RESOURCE_ROOT/shaders/SSAA.frag.set \
    $$RESOURCE_ROOT/shaders/SSAO.frag \
    $$RESOURCE_ROOT/shaders/SSAO.frag.set \

materials.path = $$resources.path/materials
materials.files =\
    $$RESOURCE_ROOT/materials/DefaultMesh.mtl \
    $$RESOURCE_ROOT/materials/DefaultMesh.mtl.set \
    $$RESOURCE_ROOT/materials/DefaultSprite.mtl \
    $$RESOURCE_ROOT/materials/DefaultSprite.mtl.set \
    $$RESOURCE_ROOT/materials/VSM.mtl \
    $$RESOURCE_ROOT/materials/VSM.mtl.set

meshes.path = $$resources.path/meshes
meshes.files =\
    $$RESOURCE_ROOT/meshes/cilinder.fbx \
    $$RESOURCE_ROOT/meshes/cilinder.fbx.set \
    $$RESOURCE_ROOT/meshes/cone.fbx \
    $$RESOURCE_ROOT/meshes/cone.fbx.set \
    $$RESOURCE_ROOT/meshes/cube.fbx \
    $$RESOURCE_ROOT/meshes/cube.fbx.set \
    $$RESOURCE_ROOT/meshes/plane.fbx \
    $$RESOURCE_ROOT/meshes/plane.fbx.set \
    $$RESOURCE_ROOT/meshes/sphere.fbx \
    $$RESOURCE_ROOT/meshes/sphere.fbx.set

code.path = $$resources.path/templates
code.files =\
    $$RESOURCE_ROOT/templates/application.cpp \
    $$RESOURCE_ROOT/templates/plugin.cpp \
    $$RESOURCE_ROOT/templates/project.qbs

textures.path = $$resources.path/textures
textures.files =\
    $$RESOURCE_ROOT/textures/invalid.png \
    $$RESOURCE_ROOT/textures/invalid.png.set

include.path = $$OUT_PWD/../$${SDK_PATH}/include

nextinc.path = $$include.path/next
nextinc.files = \
    ../$${NEXT_INCLUDE}/*

engineinc.path = $$include.path/engine
engineinc.files = \
    ../engine/includes/module.h \
    ../engine/includes/system.h \
    ../engine/includes/controller.h \
    ../engine/includes/engine.h \
    ../engine/includes/input.h \
    ../engine/includes/timer.h \
    ../engine/includes/file.h \
    ../engine/includes/log.h \
    ../engine/includes/platform.h \
    ../engine/includes/components/actor.h \
    ../engine/includes/components/camera.h \
    ../engine/includes/components/component.h \
    ../engine/includes/components/sprite.h \
    ../engine/includes/components/staticmesh.h \
    ../engine/includes/resources/mesh.h \
    ../engine/includes/resources/shader.h \
    ../engine/includes/resources/texture.h \
    ../engine/includes/resources/material.h \
    ../modules/renders/rendergl/includes/rendergl.h

lib.path = $$OUT_PWD/../$${SDK_PATH}/lib/$${PLATFORM}

next_shared.path = $$bin.path
next_shared.files = \
    ../$${NEXT_SHARED}/*.lib

next_static.path = $$lib.path
next_static.files = \
    ../$${NEXT_STATIC}/*.lib

pysfs_shared.path = $$bin.path
pysfs_shared.files = \
    ../$${PHYSFS_SHARED}/*.lib

pysfs_static.path = $$lib.path
pysfs_static.files = \
    ../$${PHYSFS_STATIC}/*.lib

zlib_shared.path = $$bin.path
zlib_shared.files = \
    ../$${ZLIB_SHARED}/*.lib

zlib_static.path = $$lib.path
zlib_static.files = \
    ../$${ZLIB_STATIC}/*.lib

glfwlib.path = $$lib.path
glfwlib.files = \
    ../$${GLFW_LIBRARY}/glfw3.lib

glewlib.path = $$lib.path
glewlib.files = \
    ../$${GLEW_LIBRARY}/glew32s.lib

INSTALLS += \
    bin \
    qbs \
    qt \
    platforms \
    imageformats \
    shaders \
    materials \
    meshes \
    textures \
    code \
    engineinc \
    nextinc \
    next_shared \
    next_static \
    pysfs_shared \
    pysfs_static \
    zlib_shared \
    zlib_static \
    glfwlib \
    glewlib

FORMS += \
    src/editors/componentbrowser/componentbrowser.ui \
    src/editors/contentbrowser/contentbrowser.ui \
    src/editors/contentbrowser/contentselect.ui \
    src/editors/materialedit/materialedit.ui \
    src/editors/meshedit/meshedit.ui \
    src/editors/meshedit/skeletalmeshedit.ui \
    src/editors/meshedit/staticmeshedit.ui \
    src/editors/objecthierarchy/hierarchybrowser.ui \
    src/editors/propertyedit/editors/StringEdit.ui \
    src/editors/propertyedit/propertyeditor.ui \
    src/editors/scenecomposer/scenecomposer.ui \
    src/editors/textureedit/textureedit.ui \
    src/managers/asseteditormanager/importqueue.ui \
    src/managers/consolemanager/consolemanager.ui \
    src/managers/pluginmanager/plugindialog.ui \
    src/editors/propertyedit/editors/Actions.ui

HEADERS += \
    src/controllers/cameractrl.h \
    src/controllers/objectctrl.h \
    src/controllers/widgetctrl.h \
    src/editors/componentbrowser/componentbrowser.h \
    src/editors/componentbrowser/componentmodel.h \
    src/editors/contentbrowser/contentbrowser.h \
    src/editors/contentbrowser/contentlist.h \
    src/editors/contentbrowser/contentselect.h \
    src/editors/contentbrowser/contenttree.h \
    src/editors/materialedit/materialedit.h \
    src/editors/meshedit/meshedit.h \
    src/editors/objecthierarchy/hierarchybrowser.h \
    src/editors/objecthierarchy/objecthierarchymodel.h \
    src/editors/propertyedit/custom/AssetProperty.h \
    src/editors/propertyedit/custom/BoolProperty.h \
    src/editors/propertyedit/custom/ColorProperty.h \
    src/editors/propertyedit/custom/EnumProperty.h \
    src/editors/propertyedit/custom/FilePathProperty.h \
    src/editors/propertyedit/custom/Property.h \
    src/editors/propertyedit/custom/StringProperty.h \
    src/editors/propertyedit/custom/Vector2DProperty.h \
    src/editors/propertyedit/custom/Vector3DProperty.h \
    src/editors/propertyedit/editors/ArrayEdit.h \
    src/editors/propertyedit/editors/ColorEdit.h \
    src/editors/propertyedit/editors/PathEdit.h \
    src/editors/propertyedit/editors/StringEdit.h \
    src/editors/propertyedit/editors/VectorEdit.h \
    src/editors/propertyedit/nextobject.h \
    src/editors/propertyedit/propertyeditor.h \
    src/editors/propertyedit/propertymodel.h \
    src/editors/scenecomposer/scenecomposer.h \
    src/editors/textureedit/textureedit.h \
    src/graph/editors/chartview.h \
    src/graph/editors/componenteditor.h \
    src/graph/editors/curveeditor.h \
    src/graph/editors/resultview.h \
    src/graph/graphwidget.h \
    src/graph/handles.h \
    src/graph/sceneview.h \
    src/managers/asseteditormanager/iconrender.h \
    src/managers/asseteditormanager/importqueue.h \
    src/managers/consolemanager/consolemanager.h \
    src/managers/consolemanager/logmodel.h \
    src/managers/pluginmanager/plugindelegate.h \
    src/managers/pluginmanager/plugindialog.h \
    src/managers/pluginmanager/pluginmodel.h \
    src/managers/toolwindowmanager/private/qtoolwindowmanager_p.h \
    src/managers/toolwindowmanager/private/qtoolwindowmanagerarea_p.h \
    src/managers/toolwindowmanager/private/qtoolwindowmanagerwrapper_p.h \
    src/managers/toolwindowmanager/qabstracttoolwindowmanagerarea.h \
    src/managers/toolwindowmanager/qtoolwindowmanager.h \
    src/managers/undomanager/undomanager.h \
    src/qlog.h \
    src/settings.h \
    ../develop/managers/assetmanager/include/material/aconstvalue.h \
    ../develop/managers/assetmanager/include/material/acoordinates.h \
    ../develop/managers/assetmanager/include/material/agradient.h \
    ../develop/managers/assetmanager/include/material/amaterialparam.h \
    ../develop/managers/assetmanager/include/material/amathfunction.h \
    ../develop/managers/assetmanager/include/material/amathoperator.h \
    ../develop/managers/assetmanager/include/material/atexturesample.h \
    ../develop/managers/assetmanager/include/material/autils.h \
    ../develop/managers/assetmanager/include/assetmanager.h \
    ../develop/managers/assetmanager/include/baseconvertersettings.h \
    ../develop/managers/assetmanager/include/fbxconverter.h \
    ../develop/managers/assetmanager/include/materialconverter.h \
    ../develop/managers/assetmanager/include/shaderbuilder.h \
    ../develop/managers/assetmanager/include/textconverter.h \
    ../develop/managers/assetmanager/include/textureconverter.h \
    ../develop/managers/assetmanager/include/textureimportsettings.h \
    ../develop/managers/codemanager/include/codemanager.h \
    ../develop/managers/codemanager/include/ibuilder.h \
    ../develop/managers/codemanager/include/qbsbuilder.h \
    ../develop/managers/projectmanager/include/projectmanager.h \
    ../develop/models/abstractschememodel.h \
    src/editors/scemeeditor/schemeeditor.h \
    ../develop/managers/assetmanager/include/functionmodel.h \
    ../develop/models/include/abstractschememodel.h \
    ../develop/models/include/baseobjectmodel.h \
    bin/templates/plugin.tpl \
    src/editors/propertyedit/editors/Actions.h \
    src/graph/handletools.h

SOURCES += \
    src/controllers/cameractrl.cpp \
    src/controllers/objectctrl.cpp \
    src/controllers/widgetctrl.cpp \
    src/editors/componentbrowser/componentbrowser.cpp \
    src/editors/componentbrowser/componentmodel.cpp \
    src/editors/contentbrowser/contentbrowser.cpp \
    src/editors/contentbrowser/contentlist.cpp \
    src/editors/contentbrowser/contentselect.cpp \
    src/editors/contentbrowser/contenttree.cpp \
    src/editors/materialedit/materialedit.cpp \
    src/editors/meshedit/meshedit.cpp \
    src/editors/objecthierarchy/hierarchybrowser.cpp \
    src/editors/objecthierarchy/objecthierarchymodel.cpp \
    src/editors/propertyedit/custom/AssetProperty.cpp \
    src/editors/propertyedit/custom/BoolProperty.cpp \
    src/editors/propertyedit/custom/ColorProperty.cpp \
    src/editors/propertyedit/custom/EnumProperty.cpp \
    src/editors/propertyedit/custom/FilePathProperty.cpp \
    src/editors/propertyedit/custom/Property.cpp \
    src/editors/propertyedit/custom/StringProperty.cpp \
    src/editors/propertyedit/custom/Vector2DProperty.cpp \
    src/editors/propertyedit/custom/Vector3DProperty.cpp \
    src/editors/propertyedit/editors/ArrayEdit.cpp \
    src/editors/propertyedit/editors/ColorEdit.cpp \
    src/editors/propertyedit/editors/PathEdit.cpp \
    src/editors/propertyedit/editors/VectorEdit.cpp \
    src/editors/propertyedit/nextobject.cpp \
    src/editors/propertyedit/propertyeditor.cpp \
    src/editors/propertyedit/propertymodel.cpp \
    src/editors/scenecomposer/scenecomposer.cpp \
    src/editors/textureedit/textureedit.cpp \
    src/graph/editors/chartview.cpp \
    src/graph/editors/componenteditor.cpp \
    src/graph/editors/curveeditor.cpp \
    src/graph/editors/resultview.cpp \
    src/graph/graphwidget.cpp \
    src/graph/handles.cpp \
    src/graph/sceneview.cpp \
    src/managers/asseteditormanager/iconrender.cpp \
    src/managers/asseteditormanager/importqueue.cpp \
    src/managers/consolemanager/consolemanager.cpp \
    src/managers/consolemanager/logmodel.cpp \
    src/managers/pluginmanager/plugindelegate.cpp \
    src/managers/pluginmanager/plugindialog.cpp \
    src/managers/pluginmanager/pluginmodel.cpp \
    src/managers/toolwindowmanager/qabstracttoolwindowmanagerarea.cpp \
    src/managers/toolwindowmanager/qtoolwindowmanager.cpp \
    src/managers/toolwindowmanager/qtoolwindowmanagerarea.cpp \
    src/managers/toolwindowmanager/qtoolwindowmanagerwrapper.cpp \
    src/managers/undomanager/undomanager.cpp \
    src/main.cpp \
    src/settings.cpp \
    ../develop/managers/assetmanager/src/assetmanager.cpp \
    ../develop/managers/assetmanager/src/baseconvertersettings.cpp \
    ../develop/managers/assetmanager/src/fbxconverter.cpp \
    ../develop/managers/assetmanager/src/materialconverter.cpp \
    ../develop/managers/assetmanager/src/shaderbuilder.cpp \
    ../develop/managers/assetmanager/src/textconverter.cpp \
    ../develop/managers/assetmanager/src/textureconverter.cpp \
    ../develop/managers/assetmanager/src/textureimportsettings.cpp \
    ../develop/managers/codemanager/src/codemanager.cpp \
    ../develop/managers/codemanager/src/ibuilder.cpp \
    ../develop/managers/codemanager/src/qbsbuilder.cpp \
    ../develop/managers/projectmanager/src/projectmanager.cpp \
    src/editors/scemeeditor/schemeeditor.cpp \
    ../develop/managers/assetmanager/src/functionmodel.cpp \
    ../develop/models/src/baseobjectmodel.cpp \
    src/editors/propertyedit/editors/Actions.cpp \
    src/editors/propertyedit/editors/StringEdit.cpp \
    src/graph/handletools.cpp
