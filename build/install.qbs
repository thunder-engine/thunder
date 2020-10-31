import qbs
import qbs.FileInfo
import qbs.Environment

Product {
    id: install
    name: "_install"
    condition: install.desktop

    Depends { name: "cpp" }
    Depends {
        name: "Qt.core"
    }

    property string suffix: {
        if(qbs.targetOS.contains("windows")) {
            return ".dll"
        } else if(qbs.targetOS.contains("darwin")) {
            return ".dylib"
        }
        return ".so"
    }

    property string QTPLUGINS_PATH: install.PLATFORM_PATH + "/" + install.bundle + "/plugins"
    property string QML_PATH: install.PLATFORM_PATH + "/" + install.bundle + "/qml"

    property var pluginFiles: {
        var files = []
        if(qbs.targetOS.contains("windows")) {
            if(qbs.debugInformation) {
                files.push("**/*d.dll")
            } else {
                files.push("**/*.dll")
            }
        } else if(qbs.targetOS.contains("linux")) {
            files.push("**/*.so")
        } else {
            files.push("*")
        }
        return files
    }

    property var pluginExcludeFiles: {
        var files = ["*.pdb"];
        if (!(qbs.targetOS.contains("windows") && qbs.debugInformation)) {
            files.push("**/*d.dll");
        }
        return files;
    }

    Group {
        name: "Qt DLLs"
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
                var libPostfix = ((qbs.targetOS.contains("windows") && qbs.debugInformation) ? "d": "") + cpp.dynamicLibrarySuffix
                var libs = ["Qt5Core", "Qt5Gui", "Qt5Script", "Qt5Xml",
                            "Qt5XmlPatterns", "Qt5Network", "Qt5Multimedia",
                            "Qt5QuickWidgets", "Qt5Quick", "Qt5QuickTemplates2", "Qt5QuickShapes",
                            "Qt5QuickControls2", "Qt5Qml", "Qt5Svg", "Qt5Widgets"]
                if(qbs.targetOS.contains("linux")) {
                    for(var it in libs) {
                        list.push(libPrefix + libs[it] + libPostfix + "." + Qt.core.versionMajor + "." + Qt.core.versionMinor + "." + Qt.core.versionPatch)
                        list.push(libPrefix + libs[it] + libPostfix + "." + Qt.core.versionMajor + "." + Qt.core.versionMinor)
                        list.push(libPrefix + libs[it] + libPostfix + "." + Qt.core.versionMajor)
                    }

                    list.push("libicudata.so.56", "libicudata.so.56.1")
                    list.push("libicui18n.so.56", "libicui18n.so.56.1")
                    list.push("libicuuc.so.56", "libicuuc.so.56.1")

                    list.push(libPrefix + "Qt5DBus" + libPostfix + "." + Qt.core.versionMajor + "." + Qt.core.versionMinor + "." + Qt.core.versionPatch)
                    list.push(libPrefix + "Qt5DBus" + libPostfix + "." + Qt.core.versionMajor + "." + Qt.core.versionMinor)
                    list.push(libPrefix + "Qt5DBus" + libPostfix + "." + Qt.core.versionMajor)

                    list.push(libPrefix + "Qt5XcbQpa" + libPostfix + "." + Qt.core.versionMajor + "." + Qt.core.versionMinor + "." + Qt.core.versionPatch)
                    list.push(libPrefix + "Qt5XcbQpa" + libPostfix + "." + Qt.core.versionMajor + "." + Qt.core.versionMinor)
                    list.push(libPrefix + "Qt5XcbQpa" + libPostfix + "." + Qt.core.versionMajor)
                } else {
                    for(var it in libs) {
                        list.push(libPrefix + libs[it] + libPostfix)
                    }
                }
            } else {
                list.push("**/QtCore.framework/**")
                list.push("**/QtGui.framework/**")
                list.push("**/QtWidgets.framework/**")
                list.push("**/QtScript.framework/**")
                list.push("**/QtXml.framework/**")
                list.push("**/Qt5XmlPatterns.framework/**")
                list.push("**/QtNetwork.framework/**")
                list.push("**/QtMultimedia.framework/**"),
                list.push("**/QtQml.framework/**")
                list.push("**/QtQuick.framework/**")
                list.push("**/Qt5QuickTemplates2.framework/**")
                list.push("**/Qt5QuickControls2.framework/**")
                list.push("**/QtQuickWidgets.framework/**")
                list.push("**/Qt5Svg.framework/**")
                list.push("**/QtPrintSupport.framework/**")
                list.push("**/QtDBus.framework/**")
            }
            return list
        }
        qbs.install: install.desktop
        qbs.installDir: {
            if(qbs.targetOS.contains("darwin")) {
                return install.BIN_PATH + "/" + install.bundle + "../Frameworks/"
            } else if(qbs.targetOS.contains("windows")) {
                return install.BIN_PATH + "/" + install.bundle
            }
            return install.LIB_PATH
        }
        qbs.installPrefix: install.PREFIX

        excludeFiles: [
            "**/Headers",
            "**/Headers/**",
            "**/*.prl",
            "**/*_debug"
        ]
        qbs.installSourceBase: prefix
    }

    Group {
        name: "Qt Image Format Plugins"
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/imageformats/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.QTPLUGINS_PATH + "/imageformats"
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "Qt Platform Plugins"
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/platforms/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.QTPLUGINS_PATH + "/platforms"
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "Qt XCB Integrations Plugins"
        condition: qbs.targetOS.contains("linux")
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/xcbglintegrations/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.QTPLUGINS_PATH + "/xcbglintegrations"
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "Qt Config"
        condition: (qbs.targetOS.contains("windows") || qbs.targetOS.contains("linux"))
        files: install.RESOURCE_ROOT + "/qt.conf"
        qbs.install: true
        qbs.installDir: install.BIN_PATH + "/" + install.bundle
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "QML Plugins"
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/../qml/")
        files: [
            "QtGraphicalEffects/**",
            "QtQuick/Controls.2/**",
            "QtQuick/Shapes/**",
            "QtQuick/Templates.2/**",
            "QtQuick/XmlListModel/**",
            "QtQuick/Layouts/**",
            "QtQuick/Window.2/**",
            "QtQuick.2/**"
        ]
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.QML_PATH
        qbs.installPrefix: install.PREFIX
        qbs.installSourceBase: prefix
    }

    Group {
        name: "Runtime DLLs"
        condition: qbs.targetOS.contains("windows")

        property string vspath: {
            var result  = Environment.getEnv("VS140COMNTOOLS") + "../../VC/redist"
            var type    = ""
            if(qbs.debugInformation) {
                result += "/Debug_NonRedist"
                type    = "Debug"
            }
            var arch = "x86"
            if(qbs.architecture !== "x86") {
                arch = "x64"
            }
            result += "/" + arch + "/Microsoft.VC140." + type + "CRT"

            return result;
        }

        files: [
            vspath + "/msvcp140" + ((qbs.debugInformation) ? "d" : "") + ".dll"
        ]
        qbs.install: true
        qbs.installDir: install.BIN_PATH + "/" + install.bundle
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "OpenAL Binary"
        condition: qbs.targetOS.contains("windows")
        files: [
            "../thirdparty/openal/windows/bin/OpenAL32.dll"
        ]
        qbs.install: true
        qbs.installDir: install.BIN_PATH + "/" + install.bundle
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "Shaders Engine"
        files: [
            install.RESOURCE_ROOT + "/engine/shaders/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/engine/shaders"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Shaders Editor"
        files: [
            install.RESOURCE_ROOT + "/editor/shaders/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/editor/shaders"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Materials Engine"
        files: [
            install.RESOURCE_ROOT + "/engine/materials/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/engine/materials"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Materials Editor"
        files: [
            install.RESOURCE_ROOT + "/editor/materials/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/editor/materials"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Meshes Engine"
        files: [
            install.RESOURCE_ROOT + "/engine/meshes/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/engine/meshes"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Meshes Editor"
        files: [
            install.RESOURCE_ROOT + "/editor/meshes/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/editor/meshes"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Templates Editor"
        files: [
            install.RESOURCE_ROOT + "/editor/templates/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/editor/templates"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Textures Engine"
        files: [
            install.RESOURCE_ROOT + "/engine/textures/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/engine/textures"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Textures Editor"
        files: [
            install.RESOURCE_ROOT + "/editor/textures/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/editor/textures"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Next includes"
        prefix: "../thirdparty/next/inc/"
        files: [
            "**"
        ]
        qbs.install: true
        qbs.installDir: install.INC_PATH + "/next"
        qbs.installPrefix: install.PREFIX
        qbs.installSourceBase: prefix
    }
    Group {
        name: "Engine includes"
        prefix: "../engine/includes/"
        files: [
            "**/*.h"
        ]
        excludeFiles: [
            "adapters/*.h",
            "handles/*.h",
            "systems/*.h"
        ]
        qbs.install: true
        qbs.installDir: install.INC_PATH + "/engine"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "RenderGL includes"
        prefix: "../modules/renders/rendergl/includes/"
        files: [
            "**/rendergl.h"
        ]
        qbs.install: true
        qbs.installDir: install.INC_PATH + "/modules"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Angel includes"
        prefix: "../modules/vms/angel/includes/"
        files: [
            "**/angel.h"
        ]
        qbs.install: true
        qbs.installDir: install.INC_PATH + "/modules"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Media includes"
        prefix: "../modules/media/includes/"
        files: [
            "**/media.h"
        ]
        qbs.install: true
        qbs.installDir: install.INC_PATH + "/modules"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Bullet includes"
        prefix: "../modules/physics/bullet/includes/"
        files: [
            "**/bullet.h"
        ]
        qbs.install: true
        qbs.installDir: install.INC_PATH + "/modules"
        qbs.installPrefix: install.PREFIX
    }
}
