include(../../../_Paths.pri)

TEMPLATE = lib
CONFIG += shared_and_static build_all

CONFIG(shared, shared|static) {
    DEFINES += NEXT_LIBRARY BUILD_SHARED
    TARGET = ../../../../$${SDK_PATH}/bin/$${PLATFORM}/rendergl

    LIBS += \
        -L../../../$${SDK_PATH}/bin/$${PLATFORM} \
        -L../../../$${NEXT_SHARED} \
        -L../../../$${GLEW_LIBRARY} \
        -lengine$${LIB_PREFIX} \
        -lnext \
        -lglew32 \
        -lopengl32
} else {
    DEFINES += GLEW_STATIC
    TARGET = ../../../../$${SDK_PATH}/lib/$${PLATFORM}/rendergl
}

CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}

SOURCES += \
    src/components/acameragl.cpp \
    src/components/alightsourcegl.cpp \
    src/resources/amaterialgl.cpp \
    src/components/aspritegl.cpp \
    src/components/astaticmeshgl.cpp \
    src/postprocess/abloomgl.cpp \
    src/resources/ameshgl.cpp \
    src/resources/atexturegl.cpp \
    src/adecal.cpp \
    src/adeferredshading.cpp \
    src/apipeline.cpp \
    src/postprocess/aantialiasinggl.cpp \
    src/postprocess/aambientocclusiongl.cpp \
    src/filters/ablurgl.cpp \
    src/renderglsystem.cpp \
    src/rendergl.cpp

HEADERS += \
    includes/components/acameragl.h \
    includes/components/alightsourcegl.h \
    includes/resources/amaterialgl.h \
    includes/components/aspritegl.h \
    includes/components/astaticmeshgl.h \
    includes/postprocess/abloomgl.h \
    includes/resources/ameshgl.h \
    includes/resources/atexturegl.h \
    includes/adecal.h \
    includes/adeferredshading.h \
    includes/agl.h \
    includes/apipeline.h \
    includes/postprocess/aantialiasinggl.h \
    includes/postprocess/aambientocclusiongl.h \
    includes/filters/ablurgl.h \
    includes/postprocess/apostprocessor.h \
    includes/renderglsystem.h \
    includes/rendergl.h

INCLUDEPATH += \
    includes \
    ../../../common/includes \
    ../../../engine/includes \
    ../../../$${NEXT_INCLUDE} \
    ../../../$${NEXT_INCLUDE}/math \
    ../../../$${NEXT_INCLUDE}/core \
    ../../../$${GLEW_INCLUDE}
