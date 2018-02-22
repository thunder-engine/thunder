/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qbs.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

import qbs
import qbs.Environment
import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.Probes
import qbs.TextFile
import qbs.Utilities
import "utils.js" as SdkUtils

Module {
    Probes.AndroidSdkProbe {
        id: sdkProbe
        environmentPaths: (sdkDir ? [sdkDir] : []).concat(base)
    }

    Probes.AndroidNdkProbe {
        id: ndkProbe
        sdkPath: sdkProbe.path
        environmentPaths: (ndkDir ? [ndkDir] : []).concat(base)
    }

    property path sdkDir: sdkProbe.path
    property path ndkDir: ndkProbe.path
    property path ndkSamplesDir: ndkProbe.samplesDir
    property string buildToolsVersion: sdkProbe.buildToolsVersion
    property var buildToolsVersionParts: buildToolsVersion ? buildToolsVersion.split('.').map(function(item) { return parseInt(item, 10); }) : []
    property int buildToolsVersionMajor: buildToolsVersionParts[0]
    property int buildToolsVersionMinor: buildToolsVersionParts[1]
    property int buildToolsVersionPatch: buildToolsVersionParts[2]
    property string platform: sdkProbe.platform

    // Internal properties.
    property int platformVersion: {
        if (platform) {
            var match = platform.match(/^android-([0-9]+)$/);
            if (match !== null) {
                return parseInt(match[1], 10);
            }
        }
    }

    property string platformJavaVersion: {
        if (platformVersion >= 21)
            return "1.7";
        return "1.5";
    }

    property path buildToolsDir: FileInfo.joinPaths(sdkDir, "build-tools", buildToolsVersion)
    property path aaptFilePath: FileInfo.joinPaths(buildToolsDir, "aapt")
    property path apksignerFilePath: FileInfo.joinPaths(buildToolsDir, "apksigner")
    property path aidlFilePath: FileInfo.joinPaths(buildToolsDir, "aidl")
    property path dxFilePath: FileInfo.joinPaths(buildToolsDir, "dx")
    property path zipalignFilePath: FileInfo.joinPaths(buildToolsDir, "zipalign")
    property path androidJarFilePath: FileInfo.joinPaths(sdkDir, "platforms", platform,
                                                         "android.jar")
    property path generatedJavaFilesBaseDir: FileInfo.joinPaths(product.buildDirectory, "gen")
    property path generatedJavaFilesDir: FileInfo.joinPaths(generatedJavaFilesBaseDir,
                                         (product.packageName || "").split('.').join('/'))
    property string apkContentsDir: FileInfo.joinPaths(product.buildDirectory, "bin")
    property string debugKeyStorePath: FileInfo.joinPaths(
                                           Environment.getEnv(qbs.hostOS.contains("windows")
                                                              ? "USERPROFILE" : "HOME"),
                                           ".android", "debug.keystore")
    property bool useApksigner: Utilities.versionCompare(buildToolsVersion, "24.0.3") >= 0

    Depends { name: "java" }
    java.languageVersion: platformJavaVersion
    java.runtimeVersion: platformJavaVersion
    java.bootClassPaths: androidJarFilePath

    FileTagger {
        patterns: ["AndroidManifest.xml"]
        fileTags: ["android.manifest"]
    }

    FileTagger {
        patterns: ["*.aidl"]
        fileTags: ["android.aidl"]
    }

    FileTagger {
        patterns: ["*.keystore"]
        fileTags: ["android.keystore"]
    }

    // Typically there is a debug keystore in ~/.android/debug.keystore which gets created
    // by the native build tools the first time a build is done. However, we don't want to create it
    // ourselves, because writing to a location outside the qbs build directory is both polluting
    // and has the potential for race conditions. So we'll instruct the user what to do.
    Group {
        name: "Android debug keystore"
        files: {
            if (!File.exists(Android.sdk.debugKeyStorePath)) {
                throw ModUtils.ModuleError("Could not find an Android debug keystore at " +
                      Android.sdk.debugKeyStorePath + ". " +
                      "If you are developing for Android on this machine for the first time and " +
                      "have never built an application using the native Gradle / Android Studio " +
                      "tooling, this is normal. You must create the debug keystore now using the " +
                      "following command, in order to continue:\n\n" +
                      SdkUtils.createDebugKeyStoreCommandString(java.keytoolFilePath,
                                                                Android.sdk.debugKeyStorePath) +
                      "\n\n" +
                      "See the following URL for more information: " +
                      "https://developer.android.com/studio/publish/app-signing.html#debug-mode");
            }
            return [Android.sdk.debugKeyStorePath];
        }
        fileTags: ["android.keystore"]
    }

    Parameter {
        property bool embedJar: true
    }

    Rule {
        inputs: ["android.aidl"]
        Artifact {
            filePath: FileInfo.joinPaths(Utilities.getHash(input.filePath),
                                         input.completeBaseName + ".java")
            fileTags: ["java.java"]
        }

        prepare: {
            var aidl = ModUtils.moduleProperty(product, "aidlFilePath");
            cmd = new Command(aidl, [input.filePath, output.filePath]);
            cmd.description = "Processing " + input.fileName;
            return [cmd];
        }
    }

    Rule {
        multiplex: true
        inputs: ["android.resources", "android.assets", "android.manifest"]

        outputFileTags: ["java.java"]
        outputArtifacts: {
            var artifacts = [];
            var resources = inputs["android.resources"];
            if (resources && resources.length) {
                artifacts.push({
                    filePath: FileInfo.joinPaths(
                                  ModUtils.moduleProperty(product, "generatedJavaFilesDir"),
                                  "R.java"),
                    fileTags: ["java.java"]
                });
            }

            return artifacts;
        }

        prepare: SdkUtils.prepareAaptGenerate.apply(SdkUtils, arguments)
    }

    Rule {
        multiplex: true
        condition: !!product.packageName

        Artifact {
            filePath: FileInfo.joinPaths(ModUtils.moduleProperty(product, "generatedJavaFilesDir"),
                                         "BuildConfig.java")
            fileTags: ["java.java"]
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Generating BuildConfig.java";
            cmd.sourceCode = function() {
                var debugValue = product.moduleProperty("qbs", "buildVariant") === "debug"
                        ? "true" : "false";
                var ofile = new TextFile(output.filePath, TextFile.WriteOnly);
                ofile.writeLine("package " + product.packageName +  ";")
                ofile.writeLine("public final class BuildConfig {");
                ofile.writeLine("    public final static boolean DEBUG = " + debugValue + ";");
                ofile.writeLine("}");
                ofile.close();
            };
            return [cmd];
        }
    }

    Rule {
        multiplex: true
        inputs: ["java.class"]
        inputsFromDependencies: ["java.jar"]
        Artifact {
            filePath: FileInfo.joinPaths(product.Android.sdk.apkContentsDir, "classes.dex")
            fileTags: ["android.dex"]
        }
        prepare: SdkUtils.prepareDex.apply(SdkUtils, arguments)
    }

    Rule {
        multiplex: true
        inputsFromDependencies: [
            "android.gdbserver-info", "android.stl-info", "android.nativelibrary"
        ]
        outputFileTags: ["android.gdbserver", "android.stl", "android.nativelibrary-deployed"]
        outputArtifacts: {
            var libArtifacts = [];
            if (inputs["android.nativelibrary"]) {
                for (var i = 0; i < inputs["android.nativelibrary"].length; ++i) {
                    var inp = inputs["android.nativelibrary"][i];
                    var destDir = FileInfo.joinPaths(product.Android.sdk.apkContentsDir, "lib",
                                                     inp.moduleProperty("Android.ndk", "abi"));
                    libArtifacts.push({
                            filePath: FileInfo.joinPaths(destDir, inp.fileName),
                            fileTags: ["android.nativelibrary-deployed"]
                    });
                }
            }
            var gdbServerArtifacts = SdkUtils.outputArtifactsFromInfoFiles(inputs,
                    product, "android.gdbserver-info", "android.gdbserver");
            var stlArtifacts = SdkUtils.outputArtifactsFromInfoFiles(inputs, product,
                    "android.stl-info", "android.deployed-stl");
            return libArtifacts.concat(gdbServerArtifacts).concat(stlArtifacts);
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Pre-packaging native binaries";
            cmd.sourceCode = function() {
                if (inputs["android.nativelibrary"]) {
                    for (var i = 0; i < inputs["android.nativelibrary"].length; ++i) {
                        for (var j = 0; j < outputs["android.nativelibrary-deployed"].length; ++j) {
                            var inp = inputs["android.nativelibrary"][i];
                            var outp = outputs["android.nativelibrary-deployed"][j];
                            var inpAbi = inp.moduleProperty("Android.ndk", "abi");
                            var outpAbi = FileInfo.fileName(outp.baseDir);
                            if (inp.fileName === outp.fileName && inpAbi === outpAbi) {
                                File.copy(inp.filePath, outp.filePath);
                                break;
                            }
                        }
                    }
                }
                var pathsSpecs = SdkUtils.sourceAndTargetFilePathsFromInfoFiles(inputs, product,
                        "android.gdbserver-info");
                for (i = 0; i < pathsSpecs.sourcePaths.length; ++i)
                    File.copy(pathsSpecs.sourcePaths[i], pathsSpecs.targetPaths[i]);
                pathsSpecs = SdkUtils.sourceAndTargetFilePathsFromInfoFiles(inputs, product,
                        "android.stl-info");
                for (i = 0; i < pathsSpecs.sourcePaths.length; ++i)
                    File.copy(pathsSpecs.sourcePaths[i], pathsSpecs.targetPaths[i]);
            };
            return [cmd];
        }
    }

    Rule {
        multiplex: true
        inputs: [
            "android.resources", "android.assets", "android.manifest",
            "android.dex", "android.gdbserver", "android.stl",
            "android.nativelibrary-deployed", "android.keystore"
        ]
        Artifact {
            filePath: product.targetName + ".apk"
            fileTags: ["android.apk"]
        }
        prepare: SdkUtils.prepareAaptPackage.apply(SdkUtils, arguments)
    }
}
