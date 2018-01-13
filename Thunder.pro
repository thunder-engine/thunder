include(_Paths.pri)

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
    engine/Engine.pro \
    modules/Modules.pro

win32: {
    inno.files = \
        build/*.iss
    inno.path = $$OUT_PWD

    INSTALLS += \
        inno

    SUBDIRS += \
        builder/Builder.pro \
        worldeditor/WorldEditor.pro

    innoconfig.input = build/InnoSetup.iss.in
    innoconfig.output = InnoSetup.iss

    QMAKE_SUBSTITUTES += \
            innoconfig
}
