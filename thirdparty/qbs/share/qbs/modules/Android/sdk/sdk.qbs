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

import qbs.Environment
import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.Probes
import qbs.TextFile
import qbs.Utilities
import qbs.Xml
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

    // Product-specific properties and files
    property string packageName: product.name
    property string apkBaseName: packageName
    property bool automaticSources: true
    property bool legacyLayout: false
    property string sourceSetDir: legacyLayout
                                ? product.sourceDirectory
                                : FileInfo.joinPaths(product.sourceDirectory, "src/main")
    property string resourcesDir: FileInfo.joinPaths(sourceSetDir, "res")
    property string assetsDir: FileInfo.joinPaths(sourceSetDir, "assets")
    property string sourcesDir: FileInfo.joinPaths(sourceSetDir, legacyLayout ? "src" : "java")
    property string manifestFile: defaultManifestFile
    readonly property string defaultManifestFile: FileInfo.joinPaths(sourceSetDir,
                                                                   "AndroidManifest.xml")

    property bool _enableRules: !product.multiplexConfigurationId && !!packageName

    Group {
        name: "java sources"
        condition: Android.sdk.automaticSources
        prefix: Android.sdk.sourcesDir + '/'
        files: "**/*.java"
    }

    Group {
        name: "android resources"
        condition: Android.sdk.automaticSources
        fileTags: ["android.resources"]
        prefix: Android.sdk.resourcesDir + '/'
        files: "**/*"
    }

    Group {
        name: "android assets"
        condition: Android.sdk.automaticSources
        fileTags: ["android.assets"]
        prefix: Android.sdk.assetsDir + '/'
        files: "**/*"
    }

    Group {
        name: "manifest"
        condition: Android.sdk.automaticSources
        fileTags: ["android.manifest"]
        files: Android.sdk.manifestFile
               && Android.sdk.manifestFile !== Android.sdk.defaultManifestFile
               ? [Android.sdk.manifestFile]
               : (File.exists(Android.sdk.defaultManifestFile)
                  ? [Android.sdk.defaultManifestFile] : [])
    }


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
    property path frameworkAidlFilePath: FileInfo.joinPaths(sdkDir, "platforms", platform,
                                                            "framework.aidl")
    property path generatedJavaFilesBaseDir: FileInfo.joinPaths(product.buildDirectory, "gen")
    property path generatedJavaFilesDir: FileInfo.joinPaths(generatedJavaFilesBaseDir,
                                         (packageName || "").split('.').join('/'))
    property string apkContentsDir: FileInfo.joinPaths(product.buildDirectory, "bin")
    property string debugKeyStorePath: FileInfo.joinPaths(
                                           Environment.getEnv(qbs.hostOS.contains("windows")
                                                              ? "USERPROFILE" : "HOME"),
                                           ".android", "debug.keystore")
    property bool useApksigner: buildToolsVersion && Utilities.versionCompare(
                                    buildToolsVersion, "24.0.3") >= 0
    property stringList aidlSearchPaths

    Depends { name: "java"; condition: _enableRules }
    Properties {
        condition: _enableRules
        java.languageVersion: platformJavaVersion
        java.runtimeVersion: platformJavaVersion
        java.bootClassPaths: androidJarFilePath
    }

    validate: {
        if (!sdkDir) {
            throw ModUtils.ModuleError("Could not find an Android SDK at any of the following "
                                       + "locations:\n\t" + sdkProbe.candidatePaths.join("\n\t")
                                       + "\nInstall the Android SDK to one of the above locations, "
                                       + "or set the Android.sdk.sdkDir property or "
                                       + "ANDROID_HOME environment variable to a valid "
                                       + "Android SDK location.");
        }
    }

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
        condition: _enableRules
        inputs: ["android.aidl"]
        Artifact {
            filePath: FileInfo.joinPaths(Utilities.getHash(input.filePath),
                                         input.completeBaseName + ".java")
            fileTags: ["java.java"]
        }

        prepare: {
            var aidl = product.Android.sdk.aidlFilePath;
            var args = ["-p" + product.Android.sdk.frameworkAidlFilePath];
            var aidlSearchPaths = input.Android.sdk.aidlSearchPaths;
            for (var i = 0; i < (aidlSearchPaths ? aidlSearchPaths.length : 0); ++i)
                args.push("-I" + aidlSearchPaths[i]);
            args.push(input.filePath, output.filePath);
            cmd = new Command(aidl, args);
            cmd.description = "Processing " + input.fileName;
            return [cmd];
        }
    }

    property bool customManifestProcessing: false
    Group {
        condition: !Android.sdk.customManifestProcessing
        fileTagsFilter: "android.manifest_processed"
        fileTags: "android.manifest_final"
    }
    Rule {
        condition: _enableRules
        inputs: "android.manifest"
        Artifact {
            filePath: FileInfo.joinPaths("processed_manifest", input.fileName)
            fileTags: "android.manifest_processed"
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Ensuring correct package name in Android manifest file";
            cmd.sourceCode = function() {
                var manifestData = new Xml.DomDocument();
                manifestData.load(input.filePath);
                var rootElem = manifestData.documentElement();
                if (!rootElem || !rootElem.isElement() || rootElem.tagName() != "manifest")
                    throw "No manifest tag found in '" + input.filePath + "'.";

                // Quick sanity check. Don't try to be fancy; let's not risk rejecting valid names.
                var packageName = product.Android.sdk.packageName;
                if (!packageName.match(/^[^.]+(?:\.[^.]+)+$/)) {
                    throw "Package name '" + packageName + "' is not valid. Please set "
                            + "Android.sdk.packageName to a name following the "
                            + "'com.mycompany.myproduct' pattern."
                }
                rootElem.setAttribute("package", packageName);

                manifestData.save(output.filePath, 4);
            }
            return cmd;
        }
    }

    Rule {
        condition: _enableRules
        multiplex: true
        inputs: ["android.resources", "android.assets", "android.manifest_final"]

        outputFileTags: ["java.java"]
        outputArtifacts: {
            var artifacts = [];
            var resources = inputs["android.resources"];
            if (resources && resources.length) {
                artifacts.push({
                    filePath: FileInfo.joinPaths(product.Android.sdk.generatedJavaFilesDir,
                                                 "R.java"),
                    fileTags: ["java.java"]
                });
            }

            return artifacts;
        }

        prepare: SdkUtils.prepareAaptGenerate.apply(SdkUtils, arguments)
    }

    Rule {
        condition: _enableRules
        multiplex: true

        Artifact {
            filePath: FileInfo.joinPaths(product.Android.sdk.generatedJavaFilesDir,
                                         "BuildConfig.java")
            fileTags: ["java.java"]
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Generating BuildConfig.java";
            cmd.sourceCode = function() {
                var debugValue = product.qbs.buildVariant === "debug" ? "true" : "false";
                var ofile = new TextFile(output.filePath, TextFile.WriteOnly);
                ofile.writeLine("package " + product.Android.sdk.packageName +  ";")
                ofile.writeLine("public final class BuildConfig {");
                ofile.writeLine("    public final static boolean DEBUG = " + debugValue + ";");
                ofile.writeLine("}");
                ofile.close();
            };
            return [cmd];
        }
    }

    Rule {
        condition: _enableRules
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
        condition: _enableRules
        property stringList inputTags: "android.nativelibrary"
        inputsFromDependencies: inputTags
        inputs: product.aggregate ? [] : inputTags
        Artifact {
            filePath: FileInfo.joinPaths(product.Android.sdk.apkContentsDir, "lib",
                                         input.Android.ndk.abi, input.fileName)
            fileTags: "android.nativelibrary_deployed"
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "copying " + input.fileName + " for packaging";
            cmd.sourceCode = function() { File.copy(input.filePath, output.filePath); };
            return cmd;
        }
    }

    Rule {
        condition: _enableRules
        multiplex: true
        property stringList inputTags: "android.gdbserver"
        inputsFromDependencies: inputTags
        inputs: product.aggregate ? [] : inputTags
        outputFileTags: "android.gdbserver_deployed"
        outputArtifacts: {
            var deploymentData = SdkUtils.gdbserverOrStlDeploymentData(product, inputs,
                                                                       "gdbserver");
            var outputs = [];
            for (i = 0; i < deploymentData.outputFilePaths.length; ++i) {
                outputs.push({filePath: deploymentData.outputFilePaths[i],
                              fileTags: "android.gdbserver_deployed"});
            }
            return outputs;
        }
        prepare: {
            var cmd = new JavaScriptCommand;
            cmd.description = "deploying gdbserver binaries";
            cmd.sourceCode = function() {
                var deploymentData = SdkUtils.gdbserverOrStlDeploymentData(product, inputs,
                                                                           "gdbserver");
                for (var i = 0; i < deploymentData.uniqueInputs.length; ++i) {
                    File.copy(deploymentData.uniqueInputs[i].filePath,
                              deploymentData.outputFilePaths[i]);
                }
            };
            return cmd;
        }
    }

    Rule {
        condition: _enableRules
        multiplex: true
        property stringList inputTags: "android.stl"
        inputsFromDependencies: inputTags
        inputs: product.aggregate ? [] : inputTags
        outputFileTags: "android.stl_deployed"
        outputArtifacts: {
            var deploymentData = SdkUtils.gdbserverOrStlDeploymentData(product, inputs, "stl");
            var outputs = [];
            for (i = 0; i < deploymentData.outputFilePaths.length; ++i) {
                outputs.push({filePath: deploymentData.outputFilePaths[i],
                              fileTags: "android.stl_deployed"});
            }
            return outputs;
        }
        prepare: {
            var cmds = [];
            var deploymentData = SdkUtils.gdbserverOrStlDeploymentData(product, inputs);
            for (var i = 0; i < deploymentData.uniqueInputs.length; ++i) {
                var input = deploymentData.uniqueInputs[i];
                var stripArgs = ["--strip-unneeded", "-o", deploymentData.outputFilePaths[i],
                                 input.filePath];
                var cmd = new Command(input.cpp.stripPath, stripArgs);
                cmd.description = "deploying " + input.fileName;
                cmds.push(cmd);
            }
            return cmds;
        }
    }

    Rule {
        condition: _enableRules
        multiplex: true
        inputs: [
            "android.resources", "android.assets", "android.manifest_final",
            "android.dex", "android.gdbserver_deployed", "android.stl_deployed",
            "android.nativelibrary_deployed", "android.keystore"
        ]
        Artifact {
            filePath: product.Android.sdk.apkBaseName + ".apk"
            fileTags: "android.apk"
        }
        prepare: SdkUtils.prepareAaptPackage.apply(SdkUtils, arguments)
    }
}
