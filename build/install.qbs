import qbs
import qbs.FileInfo
import qbs.Environment

Product {
    id: install
    name: "_install"

    Depends { name: "cpp" }
    Depends { name: "Qt.core" }

    property string suffix: {
        if(qbs.targetOS.contains("windows")) {
            return ".dll";
        } else if(qbs.targetOS.contains("darwin")) {
            return ".dylib";
        }
        return ".so";
    }

    property var pluginFiles: {
        if(qbs.targetOS.contains("windows")) {
            if(qbs.debugInformation) {
                return ["*d.dll"];
            } else {
                return ["*.dll"];
            }
        } else if(qbs.targetOS.contains("linux")) {
            return ["*.so"];
        }
        return ["*"];
    }

    property var pluginExcludeFiles: {
        var files = ["*.pdb"];
        if (!(qbs.targetOS.contains("windows") && qbs.debugInformation)) {
            files.push("*d.dll");
        }
        return files;
    }

    Group {
        name: "Qt DLLs"
        condition: install.desktop
        prefix: {
            if (qbs.targetOS.contains("windows")) {
                return Qt.core.binPath + "/"
            } else {
                return Qt.core.libPath + "/"
            }
        }

        property string postfix: {
            var suffix = "";
            if (qbs.targetOS.contains("windows") && qbs.debugInformation)
                suffix += "d";
            return suffix + cpp.dynamicLibrarySuffix;
        }
        files: {
            var list = [];
            if (!Qt.core.frameworkBuild) {
                list.push(
                    "Qt5Core" + postfix,
                    "Qt5Gui" + postfix,
                    "Qt5Widgets" + postfix
                );
            } else {
                list.push("**/QtCore.framework/**");
                list.push("**/QtGui.framework/**");
                list.push("**/QtWidgets.framework/**");
            }
            return list;
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
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/imageformats/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.BIN_PATH + "/" + install.bundle + "/imageformats"
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "Qt Platform Plugins"
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/platforms/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.BIN_PATH + "/" + install.bundle + "/platforms"
        qbs.installPrefix: install.PREFIX
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
        condition: install.desktop
        files: [
            "../thirdparty/fbx/lib/libfbxsdk" + suffix
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
        name: "Textures"
        files: [
            install.RESOURCE_ROOT + "/engine/textures/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/engine/textures"
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
            "patterns/*.h"
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
