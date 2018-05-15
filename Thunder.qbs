import qbs
import qbs.Process
import qbs.TextFile

Project {
    id: thunder

    property string COMPANY_NAME: "FrostSpear"
    property string PRODUCT_NAME: "Thunder Engine"
    property string EDITOR_NAME: "WorldEditor"
    property string BUILDER_NAME: "Builder"
    property string COPYRIGHT_AUTHOR: "Evgeniy Prikazchikov"

    property string PLATFORM: {
        var arch = qbs.architecture;
        if(qbs.targetOS.contains("darwin") || qbs.targetOS.contains("linux")) {
            arch = "x86_64"
        }
        return qbs.targetOS[0] + "/" + arch;
    }

    property bool desktop: !qbs.targetOS.contains("android") && !qbs.targetOS.contains("ios") && !qbs.targetOS.contains("tvos")
    property string bundle: {
        if(qbs.targetOS.contains("darwin")) {
            return EDITOR_NAME + ".app/Contents/MacOS/"
        }
        return "";
    }

    Probe {
        id: probe
        property string SDK_VERSION
        property string REVISION
        property string LEGAL
        property string SPONSORS
        property string YEAR
        configure: {
            YEAR = new Date().getFullYear().toString()
            REVISION = "develop"
            var p = new Process()
            p.setWorkingDirectory(thunder.sourceDirectory)
            if (p.exec("git", ["rev-parse", "HEAD"]) === 0) {
                REVISION = p.readStdOut().trim()
            } else {
                console.error(p.readStdErr())
            }
            LEGAL = new TextFile(thunder.sourceDirectory + "/legal").readAll()
            SPONSORS = new TextFile(thunder.sourceDirectory + "/sponsors").readAll()
            SDK_VERSION = new TextFile(thunder.sourceDirectory + "/version").readAll()
        }
    }

    property string COPYRIGHT_YEAR: probe.YEAR

    property string RESOURCE_ROOT: "../worldeditor/bin"

    property string PREFIX: ""
    property string LAUNCHER_PATH: "launcher"
    property string SDK_PATH: "sdk/" + probe.SDK_VERSION
    property string PLATFORM_PATH: SDK_PATH + "/" + PLATFORM
    property string BIN_PATH: PLATFORM_PATH + "/bin"
    property string LIB_PATH: PLATFORM_PATH + "/lib"
    property string INC_PATH: SDK_PATH + "/include"

    property stringList defines: {
        var result  = [
            "COMPANY_NAME=\"" + COMPANY_NAME + "\"",
            "PRODUCT_NAME=\"" + PRODUCT_NAME + "\"",
            "EDITOR_NAME=\"" + EDITOR_NAME + "\"",
            "BUILDER_NAME=\"" + BUILDER_NAME + "\"",
            "SDK_VERSION=\"" + probe.SDK_VERSION + "\"",
            "COPYRIGHT_YEAR=" + COPYRIGHT_YEAR,
            "COPYRIGHT_AUTHOR=\"" + COPYRIGHT_AUTHOR + "\"",
            "REVISION=\"" + probe.REVISION + "\"",
            "LEGAL=\"" + probe.LEGAL + "\"",
            "SPONSORS=\"" + probe.SPONSORS + "\""
        ];
        return result;
    }

    references: [
        "thirdparty/thirdparty.qbs",
        "engine/engine.qbs",
        "modules/renders/rendergl/rendergl.qbs",
        "worldeditor/worldeditor.qbs",
        "builder/builder.qbs",
        "build/install.qbs"
    ]
}

