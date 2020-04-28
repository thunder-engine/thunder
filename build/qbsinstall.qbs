import qbs
import qbs.FileInfo
import qbs.Environment

Product {
    id: qbsinstall
    name: "_qbsinstall"
    condition: qbsinstall.desktop

    Depends { name: "cpp" }
    Depends {
        name: "Qt.core"
    }

    property string qbsInstallPath: qbsinstall.TOOLS_PATH + "/qbs"

    property string qbsPath: {
        var result = "../thirdparty/qbs/release/install-root"
        if(qbs.targetOS.contains("windows")) {
            return result
        }
        return (result + "/usr/local")
    }

    Group {
        name: "QBS Bin"
        prefix: qbsPath
        files: [
            "/bin/**"
        ]
        excludeFiles: [
            "/bin/styles/**",
            "/bin/platforms/**",
            "/bin/imageformats/**",
            "/bin/bearer/**"
        ]
        qbs.install: qbsinstall.desktop
        qbs.installDir: qbsinstall.qbsInstallPath + "/bin"
        qbs.installPrefix: qbsinstall.PREFIX
    }
    Group {
        name: "QBS Lib"
        prefix: qbsPath
        files: [
            "/lib/*"
        ]
        qbs.install: qbsinstall.desktop
        qbs.installDir: qbsinstall.qbsInstallPath + "/lib"
        qbs.installPrefix: qbsinstall.PREFIX
    }
    Group {
        name: "QBS Plugins"
        prefix: qbsPath
        files: [
            "/lib/qbs/**",
            "/libexec/**"
        ]
        qbs.install: qbsinstall.desktop
        qbs.installDir: qbsinstall.qbsInstallPath
        qbs.installPrefix: qbsinstall.PREFIX
        qbs.installSourceBase: prefix
    }
    Group {
        name: "QBS Share"
        prefix: qbsPath
        files: [
            "/share/**"
        ]
        excludeFiles: [
            "/share/**/*.ts"
        ]
        qbs.install: qbsinstall.desktop
        qbs.installDir: qbsinstall.qbsInstallPath
        qbs.installPrefix: qbsinstall.PREFIX
        qbs.installSourceBase: prefix
    }
}
