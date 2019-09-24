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

    property string qbsPath: qbsinstall.TOOLS_PATH + "/qbs"

    Group {
        name: "QBS Bin"
        prefix: "../thirdparty/qbs/" + qbs.targetOS[0]
        files: [
            "/bin/**"
        ]
        qbs.install: qbsinstall.desktop
        qbs.installDir: qbsinstall.qbsPath + "/bin"
        qbs.installPrefix: qbsinstall.PREFIX
    }
    Group {
        name: "QBS Lib"
        prefix: "../thirdparty/qbs/" + qbs.targetOS[0]
        files: [
            "/lib/*"
        ]
        qbs.install: qbsinstall.desktop
        qbs.installDir: qbsinstall.qbsPath + "/lib"
        qbs.installPrefix: qbsinstall.PREFIX
    }
    Group {
        name: "QBS Plugins"
        prefix: "../thirdparty/qbs/" + qbs.targetOS[0]
        files: [
            "/lib/qbs/**",
            "/libexec/**"
        ]
        qbs.install: qbsinstall.desktop
        qbs.installDir: qbsinstall.qbsPath
        qbs.installPrefix: qbsinstall.PREFIX
        qbs.installSourceBase: prefix
    }
    Group {
        name: "QBS Share"
        prefix: "../thirdparty/qbs/"
        files: [
            "share/**"
        ]
        excludeFiles: [
            "share/**/*.ts"
        ]
        qbs.install: qbsinstall.desktop
        qbs.installDir: qbsinstall.qbsPath
        qbs.installPrefix: qbsinstall.PREFIX
        qbs.installSourceBase: prefix
    }
    Group {
        name: "QBS DLLs"
        prefix: {
            if (qbs.targetOS.contains("windows")) {
                return Qt.core.binPath + "/"
            } else {
                return Qt.core.libPath + "/"
            }
        }

        files: {
            var list = [];
            if (!Qt.core.frameworkBuild) {
                var libPrefix = (qbs.targetOS.contains("linux")) ? "lib" : ""
                var libPostfix = cpp.dynamicLibrarySuffix

                var libs = ["Qt5Core", "Qt5Script", "Qt5Xml", "Qt5Network"]
                if(qbs.targetOS.contains("linux")) {
                    for(var it in libs) {
                        list.push(libPrefix + libs[it] + libPostfix + "." + Qt.core.versionMajor + "." + Qt.core.versionMinor + "." + Qt.core.versionPatch)
                        list.push(libPrefix + libs[it] + libPostfix + "." + Qt.core.versionMajor + "." + Qt.core.versionMinor)
                        list.push(libPrefix + libs[it] + libPostfix + "." + Qt.core.versionMajor)
                    }
                } else {
                    for(var it in libs) {
                        list.push(libPrefix + libs[it] + libPostfix)
                    }
                }
            }
            return list
        }
        qbs.install: qbsinstall.desktop
        qbs.installDir: {
            var dir = "/bin"
            if(qbs.targetOS.contains("linux")) {
                dir = "/lib"
            }

            return qbsinstall.qbsPath + dir
        }
        qbs.installPrefix: qbsinstall.PREFIX
        qbs.installSourceBase: prefix
    }
}
