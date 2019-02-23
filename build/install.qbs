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
            return ".dll";
        } else if(qbs.targetOS.contains("darwin")) {
            return ".dylib";
        }
        return ".so";
    }

    property var pluginFiles: {
        var files = []
        if(qbs.targetOS.contains("windows")) {
            if(qbs.debugInformation) {
                files.push("**/*d.dll");
            } else {
                files.push("**/*.dll");
            }
        } else if(qbs.targetOS.contains("linux")) {
            files.push("**/*.so");
        } else {
            files.push("*");
        }
        return files;
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
        condition: !qbs.targetOS.contains("darwin")
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
                if(qbs.targetOS.contains("linux")) {
                    libPostfix += "." + Qt.core.versionMajor + "." + Qt.core.versionMinor + "." + Qt.core.versionPatch
                }

                list.push(
                    libPrefix + "Qt5Core" + libPostfix,
                    libPrefix + "Qt5Gui" + libPostfix,
                    libPrefix + "Qt5Widgets" + libPostfix,
                    libPrefix + "Qt5Script" + libPostfix,
                    libPrefix + "Qt5Xml" + libPostfix,
                    libPrefix + "Qt5Network" + libPostfix,
                    libPrefix + "Qt5Multimedia" + libPostfix,
                    libPrefix + "Qt5QuickWidgets" + libPostfix,
                    libPrefix + "Qt5Quick" + libPostfix,
                    libPrefix + "Qt5QuickTemplates2" + libPostfix,
                    libPrefix + "Qt5QuickControls2" + libPostfix,
                    libPrefix + "Qt5Qml" + libPostfix
                );
            } else {
                list.push("**/QtCore.framework/**");
                list.push("**/QtGui.framework/**");
                list.push("**/QtWidgets.framework/**");
                list.push("**/QtScript.framework/**");
                list.push("**/QtXml.framework/**");
                list.push("**/QtNetwork.framework/**");
                list.push("**/QtMultimedia.framework/**"),
                list.push("**/QtQml.framework/**");
                list.push("**/QtQuick.framework/**");
                list.push("**/Qt5QuickTemplates2.framework/**");
                list.push("**/Qt5QuickControls2.framework/**");
                list.push("**/QtQuickWidgets.framework/**");
            }
            return list
        }
        qbs.install: install.desktop
        qbs.installDir: install.BIN_PATH + "/" + install.bundle + (qbs.targetOS.contains("darwin") ? "../Frameworks/" : "")
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
        condition: !qbs.targetOS.contains("darwin")
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/imageformats/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.BIN_PATH + "/" + install.bundle + "/imageformats"
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "Qt Platform Plugins"
        condition: !qbs.targetOS.contains("darwin")
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/platforms/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.BIN_PATH + "/" + install.bundle + "/platforms"
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "QML Plugins"
        condition: !qbs.targetOS.contains("darwin")
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/../qml/")
        files: [
            "QtGraphicalEffects/**",
            "QtQuick/Controls.2/**",
            "QtQuick.2/**"
        ]
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.BIN_PATH + "/" + install.bundle + "/qml"
        qbs.installPrefix: install.PREFIX
        qbs.installSourceBase: prefix
    }

    Group {
        name: "Runtime DLLs"
        condition: qbs.targetOS.contains("windows")

        property string vspath: {
            var result  = Environment.getEnv("VS140COMNTOOLS") + "../../VC/redist";
            var type    = "";
            if(qbs.debugInformation) {
                result += "/Debug_NonRedist";
                type    = "Debug";
            }
            result += "/" + qbs.architecture + "/Microsoft.VC140." + type + "CRT";

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
        name: "FBX Binary"
        files: [
            "../thirdparty/fbx/lib/libfbxsdk" + suffix
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
            "adapters/*.h"
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
        qbs.installDir: install.INC_PATH + "/engine"
        qbs.installPrefix: install.PREFIX
    }
}
