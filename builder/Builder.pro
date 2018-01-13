include(../_Paths.pri)

QT += core

TARGET = ../../$${SDK_PATH}/bin/$${PLATFORM}/Builder
TEMPLATE = app

CONFIG  += console

DEFINES += QUAZIP_STATIC

QMAKE_CXXFLAGS_WARN_ON -= -w34100

SOURCES += \
    main.cpp \
    ../develop/managers/projectmanager/src/projectmanager.cpp \
    ../develop/managers/assetmanager/src/assetmanager.cpp \
    ../develop/managers/assetmanager/src/baseconvertersettings.cpp \
    ../develop/managers/assetmanager/src/textconverter.cpp \
    ../develop/managers/codemanager/src/codemanager.cpp \
    ../develop/managers/codemanager/src/ibuilder.cpp \
    ../develop/managers/codemanager/src/qbsbuilder.cpp \
    ../develop/managers/assetmanager/src/fbxconverter.cpp \
    ../develop/managers/assetmanager/src/materialconverter.cpp \
    ../develop/managers/assetmanager/src/shaderbuilder.cpp \
    ../develop/managers/assetmanager/src/textureconverter.cpp \
    ../develop/managers/assetmanager/src/textureimportsettings.cpp \
    ../develop/managers/assetmanager/src/functionmodel.cpp \
    ../develop/models/src/baseobjectmodel.cpp \
    builder.cpp

HEADERS +=  \
    ../develop/managers/projectmanager/include/projectmanager.h \
    ../develop/managers/assetmanager/include/assetmanager.h \
    ../develop/managers/assetmanager/include/baseconvertersettings.h \
    ../develop/managers/assetmanager/include/textconverter.h \
    ../develop/managers/codemanager/include/codemanager.h \
    ../develop/managers/codemanager/include/ibuilder.h \
    ../develop/managers/codemanager/include/qbsbuilder.h \
    ../develop/managers/assetmanager/include/material/aconstvalue.h \
    ../develop/managers/assetmanager/include/material/acoordinates.h \
    ../develop/managers/assetmanager/include/material/agradient.h \
    ../develop/managers/assetmanager/include/material/amaterialparam.h \
    ../develop/managers/assetmanager/include/material/amathfunction.h \
    ../develop/managers/assetmanager/include/material/amathoperator.h \
    ../develop/managers/assetmanager/include/material/atexturesample.h \
    ../develop/managers/assetmanager/include/material/autils.h \
    ../develop/managers/assetmanager/include/fbxconverter.h \
    ../develop/managers/assetmanager/include/materialconverter.h \
    ../develop/managers/assetmanager/include/shaderbuilder.h \
    ../develop/managers/assetmanager/include/textureconverter.h \
    ../develop/managers/assetmanager/include/textureimportsettings.h \
    ../develop/models/include/abstractschememodel.h \
    ../develop/managers/assetmanager/include/functionmodel.h \
    ../develop/models/include/baseobjectmodel.h \
    consolelog.h \
    builder.h

INCLUDEPATH += \
    ../common \
    ../engine/includes \
    ../develop/managers/codemanager/include \
    ../develop/managers/projectmanager/include \
    ../develop/managers/assetmanager/include \
    ../develop/models/include \
    ../$${NEXT_INCLUDE} \
    ../$${NEXT_INCLUDE}/math \
    ../$${NEXT_INCLUDE}/core \
    ../$${ZLIB_INCLUDE} \
    ../$${PHYSFS_INCLUDE} \
    ../$${FBX_INCLUDE} \
    ../$${QUAZIP_INCLUDE}

LIBS    += \
    -L../$${SDK_PATH}/bin/$${PLATFORM} \
    -L../$${NEXT_SHARED} \
    -L../$${ZLIB_SHARED} \
    -L../$${QUAZIP_SHARED} \
    -L../$${FBX_LIBRARY} \
    -lnext \
    -lengine$${LIB_PREFIX} \
    -lzlib \
    -lfbxsdk-2012.1

Debug:LIBS   += -lquazipd
Release:LIBS += -lquazip
