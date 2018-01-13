include(../_Paths.pri)

TEMPLATE = lib
CONFIG += shared_and_static build_all

CONFIG(shared, shared|static) {
    DEFINES += NEXT_LIBRARY BUILD_SHARED
    TARGET = ../../$${SDK_PATH}/bin/$${PLATFORM}/engine

    LIBS   += \
        -L../$${NEXT_SHARED} \
        -L../$${PHYSFS_SHARED} \
        -L../$${GLFW_LIBRARY} \
        -lnext \
        -lglfw3dll \
        -lphysfs
} else {
    TARGET = ../../$${SDK_PATH}/lib/$${PLATFORM}/engine
}

CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}

SOURCES += \
    src/components/actor.cpp \
    src/components/camera.cpp \
    src/components/chunk.cpp \
    src/components/component.cpp \
    src/components/lightsource.cpp \
    src/components/scene.cpp \
    src/components/sprite.cpp \
    src/components/staticmesh.cpp \
    src/analytics/profiler.cpp \
    src/adapters/desktopadaptor.cpp \
    src/engine.cpp \
    src/input.cpp \
    src/resources/mesh.cpp \
    src/resources/texture.cpp \
    src/timer.cpp \
    src/resources/material.cpp \
    src/resources/text.cpp \
    src/controller.cpp \
    src/file.cpp \
    src/log.cpp \
    src/rendersystem.cpp

HEADERS += \
    includes/components/actor.h \
    includes/components/camera.h \
    includes/components/chunk.h \
    includes/components/component.h \
    includes/components/lightsource.h \
    includes/components/scene.h \
    includes/components/sprite.h \
    includes/components/staticmesh.h \
    includes/engine.h \
    includes/input.h \
    includes/log.h \
    includes/adapters/desktopadaptor.h \
    includes/resources/animation.h \
    includes/resources/mesh.h \
    includes/analytics/profiler.h \
    includes/resources/texture.h \
    includes/timer.h \
    includes/resources/material.h \
    includes/adapters/iplatformadaptor.h \
    includes/resources/text.h \
    includes/components/screentext.h \
    includes/system.h \
    includes/controller.h \
    includes/module.h \
    includes/file.h \
    includes/platform.h \
    includes/rendersystem.h

INCLUDEPATH += \
    includes \
    ../common/includes \
    ../$${NEXT_INCLUDE} \
    ../$${NEXT_INCLUDE}/math \
    ../$${NEXT_INCLUDE}/core \
    ../$${GLFW_INCLUDE} \
    ../$${PHYSFS_INCLUDE}
