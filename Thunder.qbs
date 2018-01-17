import qbs

Project {
    id: thunder

    property string COMPANY_NAME: "FrostSpear"
    property string EDITOR_NAME: "WorldEditor"
    property string BUILDER_NAME: "Builder"
    property string SDK_VERSION: "1.0"
    property string LAUNCHER_VERSION: "1.0"
    property string YEAR: "2017"

    property string PLATFORM: "windows/" + qbs.architecture
    property string RESOURCE_ROOT: "../worldeditor/bin"

    property string PREFIX: "_PACKAGE"
    property string LAUNCHER_PATH: "launcher"
    property string SDK_PATH: "sdk/" + SDK_VERSION
    property string BIN_PATH: SDK_PATH + "/bin/" + PLATFORM
    property string LIB_PATH: SDK_PATH + "/lib/" + PLATFORM
    property string INC_PATH: SDK_PATH + "/include"

    references: [
        "thirdparty/thirdparty.qbs",
        "engine/engine.qbs",
        "modules/renders/rendergl/rendergl.qbs",
        "worldeditor/worldeditor.qbs",
        "builder/builder.qbs",
        "build/install.qbs"
    ]
}

