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
        prefix: {
            if (qbs.targetOS.contains("windows")) {
                return Qt.core.binPath + "/"
            } else {
                return Qt.core.libPath + "/lib"
            }
        }
        property string postfix: {
            var suffix = "";
            if (qbs.targetOS.contains("windows") && qbs.debugInformation)
                suffix += "d";
            return suffix + cpp.dynamicLibrarySuffix;
        }
        files: {
            function addQtVersions(libs) {
                var result = [];
                for (i = 0; i < libs.length; ++i) {
                    var major = libs[i] + "." + Qt.core.versionMajor;
                    var minor = major + "." + Qt.core.versionMinor;
                    var patch = minor + "." + Qt.core.versionPatch;
                    result.push(libs[i], major, minor, patch);
                }
                return result;
            }

            var list = [];

            if (!Qt.core.frameworkBuild) {
                list.push(
                    "Qt5Core" + postfix,
                    "Qt5Gui" + postfix,
                    "Qt5Network" + postfix,
                    "Qt5OpenGL" + postfix,
                    "Qt5Widgets" + postfix
                );
            }

            return list;
        }
        qbs.install: true
        qbs.installDir: install.BIN_PATH
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "Qt Image Format Plugins"
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/imageformats/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.BIN_PATH + "/imageformats"
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "Qt Platform Plugins"
        prefix: FileInfo.joinPaths(Qt.core.pluginPath, "/platforms/")
        files: pluginFiles
        excludeFiles: pluginExcludeFiles
        qbs.install: true
        qbs.installDir: install.BIN_PATH + "/platforms"
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
        qbs.installDir: install.BIN_PATH
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "FBX Binary"
        files: [
            "../thirdparty/fbx/bin/fbxsdk-2012.1" + suffix
        ]
        qbs.install: true
        qbs.installDir: install.BIN_PATH
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "Glew Binary"
        files: [
            "../thirdparty/glew/bin/glew32" + suffix
        ]
        qbs.install: true
        qbs.installDir: install.BIN_PATH
        qbs.installPrefix: install.PREFIX
    }

    Group {
        name: "Shaders"
        files: [
            install.RESOURCE_ROOT + "/shaders/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/shaders"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Materials"
        files: [
            install.RESOURCE_ROOT + "/materials/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/materials"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Meshes"
        files: [
            install.RESOURCE_ROOT + "/meshes/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/meshes"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Templates"
        files: [
            install.RESOURCE_ROOT + "/templates/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/templates"
        qbs.installPrefix: install.PREFIX
    }
    Group {
        name: "Textures"
        files: [
            install.RESOURCE_ROOT + "/textures/*"
        ]
        qbs.install: true
        qbs.installDir: install.SDK_PATH + "/resources/textures"
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
}
