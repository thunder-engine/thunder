import qbs
import qbs.FileInfo
import qbs.Environment

Product {
    id: install
    name: "_install"

    Depends { name: "cpp" }
    Depends {
        name: "Qt.core"
        condition: install.desktop
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
        condition: install.desktop && !qbs.targetOS.contains("darwin")
        prefix: {
            if (qbs.targetOS.contains("windows")) {
                return Qt.core.binPath + "/"
            } else {
                return Qt.core.libPath + "/"
            }
        }

		property string libPrefix: {
            var result = "";
            if (qbs.targetOS.contains("linux"))
                result = "lib";
            return result;
        }
		
        property string libPostfix: {
            var suffix = "";
            if(qbs.targetOS.contains("windows") && qbs.debugInformation) {
                suffix += "d";
            }
            suffix += cpp.dynamicLibrarySuffix
            if(qbs.targetOS.contains("linux")) {
                suffix += "." + Qt.core.versionMajor;
            }
            return suffix;
        }
        files: {
            var list = [];
            if (!Qt.core.frameworkBuild) {
                list.push(
                    libPrefix + "Qt5Core" + libPostfix,
                    libPrefix + "Qt5Gui" + libPostfix,
                    libPrefix + "Qt5Widgets" + libPostfix,
                    libPrefix + "Qt5Script" + libPostfix,
                    libPrefix + "Qt5Xml" + libPostfix,
                    libPrefix + "Qt5Network" + libPostfix
                );
                if(qbs.targetOS.contains("linux")) {
                    list.push(libPrefix + "Qt5Core" + libPostfix + "." + Qt.core.versionMinor + "." + Qt.core.versionPatch),
                    list.push(libPrefix + "Qt5Gui" + libPostfix + "." + Qt.core.versionMinor + "." + Qt.core.versionPatch),
                    list.push(libPrefix + "Qt5Widgets" + libPostfix + "." + Qt.core.versionMinor + "." + Qt.core.versionPatch),
                    list.push(libPrefix + "Qt5Script" + libPostfix + "." + Qt.core.versionMinor + "." + Qt.core.versionPatch),
                    list.push(libPrefix + "Qt5Xml" + libPostfix + "." + Qt.core.versionMinor + "." + Qt.core.versionPatch),
                    list.push(libPrefix + "Qt5Network" + libPostfix + "." + Qt.core.versionMinor + "." + Qt.core.versionPatch)
                }
            } else {
                list.push("**/QtCore.framework/**");
                list.push("**/QtGui.framework/**");
                list.push("**/QtWidgets.framework/**");
                list.push("**/QtScript.framework/**");
                list.push("**/QtXml.framework/**");
                list.push("**/QtNetwork.framework/**");
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
        condition: install.desktop && !qbs.targetOS.contains("darwin")
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/imageformats/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.BIN_PATH + "/" + install.bundle + "/imageformats"
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "Qt Platform Plugins"
        condition: install.desktop && !qbs.targetOS.contains("darwin")
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
        condition: install.desktop
        files: [
            install.RESOURCE_ROOT + "/engine/shaders/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/engine/shaders"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Materials Engine"
        condition: install.desktop
        files: [
            install.RESOURCE_ROOT + "/engine/materials/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/engine/materials"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Materials Editor"
        condition: install.desktop
        files: [
            install.RESOURCE_ROOT + "/editor/materials/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/editor/materials"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Meshes Engine"
        condition: install.desktop
        files: [
            install.RESOURCE_ROOT + "/engine/meshes/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/engine/meshes"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Meshes Editor"
        condition: install.desktop
        files: [
            install.RESOURCE_ROOT + "/editor/meshes/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/editor/meshes"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Templates Editor"
        condition: install.desktop
        files: [
            install.RESOURCE_ROOT + "/editor/templates/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/editor/templates"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Textures Engine"
        condition: install.desktop
        files: [
            install.RESOURCE_ROOT + "/engine/textures/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/engine/textures"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Textures Editor"
        condition: install.desktop
        files: [
            install.RESOURCE_ROOT + "/editor/textures/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/editor/textures"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Next includes"
        condition: install.desktop
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
        condition: install.desktop
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
        condition: install.desktop
        prefix: "../modules/renders/rendergl/includes/"
        files: [
            "**/rendergl.h"
        ]
        qbs.install: true
        qbs.installDir: install.INC_PATH + "/engine"
        qbs.installPrefix: install.PREFIX
    }

    property string qbsPath: install.BIN_PATH + "/" + install.bundle

    Group {
        name: "QBS Bin"
        condition: install.desktop
        prefix: "../thirdparty/qbs/" + qbs.targetOS[0]
        files: [
            "/bin/**"
        ]
        qbs.install: true
        qbs.installDir: install.qbsPath
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "QBS Lib"
        condition: install.desktop
        prefix: "../thirdparty/qbs/" + qbs.targetOS[0]
        files: [
            "/lib/*"
        ]
        qbs.install: true
        qbs.installDir: install.qbsPath;
        Properties {
            condition: qbs.targetOS.contains("darwin")
            qbs.installDir: install.qbsPath + "../Frameworks/";
        }
        Properties {
            condition: qbs.targetOS.contains("linux")
            qbs.installDir: install.qbsPath + "../lib/";
        }
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "QBS Plugins"
        condition: install.desktop
        prefix: "../thirdparty/qbs/" + qbs.targetOS[0]
        files: [
            "/lib/qbs/**",
            "/libexec/**"
        ]
        qbs.install: true
        qbs.installDir: install.qbsPath + "../"
        qbs.installPrefix: install.PREFIX
        qbs.installSourceBase: prefix
    }
    Group {
        name: "QBS Share"
        condition: install.desktop
        prefix: "../thirdparty/qbs/"
        files: [
            "share/**"
        ]
        excludeFiles: [
            "share/**/*.ts"
        ]
        qbs.install: true
        qbs.installDir: install.qbsPath + "../"
        qbs.installPrefix: install.PREFIX
        qbs.installSourceBase: prefix
    }
}
