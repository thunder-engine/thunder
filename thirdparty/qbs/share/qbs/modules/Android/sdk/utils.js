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

var File = require("qbs.File");
var FileInfo = require("qbs.FileInfo");
var Process = require("qbs.Process");
var TextFile = require("qbs.TextFile");
var Utilities = require("qbs.Utilities");

function availableBuildToolsVersions(sdkDir) {
    var re = /^([0-9]+)\.([0-9]+)\.([0-9]+)$/;
    var buildTools = File.directoryEntries(FileInfo.joinPaths(sdkDir, "build-tools"),
                                           File.Dirs | File.NoDotAndDotDot);
    var versions = [];
    for (var i = 0; i < buildTools.length; ++i) {
        var match = buildTools[i].match(re);
        if (match !== null) {
            versions.push(buildTools[i]);
        }
    }

    // Sort by version number
    versions.sort(function (a, b) {
        return Utilities.versionCompare(a, b);
    });

    return versions;
}

function prepareDex(project, product, inputs, outputs, input, output, explicitlyDependsOn) {
    var dxFilePath = product.Android.sdk.dxFilePath;
    var args = ["--dex", "--output", output.filePath, product.java.classFilesDir];

    var jarFiles = [];
    function traverseJarDeps(dep) {
        if (dep.parameters.Android && dep.parameters.Android.sdk
                && dep.parameters.Android.sdk.embedJar === false)
            return;

        var isJar = typeof dep.artifacts["java.jar"] !== "undefined";
        if (!isJar)
            return;

        dep.artifacts["java.jar"].forEach(function(artifact) {
            if (!jarFiles.contains(artifact.filePath))
                jarFiles.push(artifact.filePath);
        });
        dep.dependencies.forEach(traverseJarDeps);
    }
    product.dependencies.forEach(traverseJarDeps);

    args = args.concat(jarFiles);

    var cmd = new Command(dxFilePath, args);
    cmd.description = "Creating " + output.fileName;
    return [cmd];
}

function findParentDir(filePath, parentDirName)
{
    var lastDir;
    var currentDir = FileInfo.path(filePath);
    while (lastDir !== currentDir) {
        if (FileInfo.fileName(currentDir) === parentDirName)
            return currentDir;
        lastDir = currentDir;
        currentDir = FileInfo.path(currentDir);
    }
}

function commonAaptPackageArgs(project, product, inputs, outputs, input, output,
                               explicitlyDependsOn) {
    var manifestFilePath = inputs["android.manifest_final"][0].filePath;
    var args = ["package", "-f",
                "-M", manifestFilePath,
                "-I", product.Android.sdk.androidJarFilePath];
    var resources = inputs["android.resources"];
    var resourceDirs = [];
    if (resources) {
        for (var i = 0; i < resources.length; ++i) {
            var resDir = findParentDir(resources[i].filePath, "res");
            if (!resDir) {
                throw "File '" + resources[i].filePath + "' is tagged as an Android resource, "
                        + "but is not located under a directory called 'res'.";
            }
            if (!resourceDirs.contains(resDir))
                resourceDirs.push(resDir);
        }
    }
    for (i = 0; i < resourceDirs.length; ++i)
        args.push("-S", resourceDirs[i]);
    var assets = inputs["android.assets"];
    var assetDirs = [];
    if (assets) {
        for (i = 0; i < assets.length; ++i) {
            var assetDir = findParentDir(assets[i].filePath, "assets");
            if (!assetDir) {
                throw "File '" + assets[i].filePath + "' is tagged as an Android asset, "
                        + "but is not located under a directory called 'assets'.";
            }
            if (!assetDirs.contains(assetDir))
                assetDirs.push(assetDir);
        }
    }
    for (i = 0; i < assetDirs.length; ++i)
        args.push("-A", assetDirs[i]);
    if (product.qbs.buildVariant === "debug")
        args.push("--debug-mode");
    return args;
}

function prepareAaptGenerate(project, product, inputs, outputs, input, output,
                             explicitlyDependsOn) {
    var args = commonAaptPackageArgs.apply(this, arguments);
    args.push("--no-crunch", "-m");
    var resources = inputs["android.resources"];
    if (resources && resources.length)
        args.push("-J", ModUtils.moduleProperty(product, "generatedJavaFilesBaseDir"));
    var cmd = new Command(product.Android.sdk.aaptFilePath, args);
    cmd.description = "Processing resources";
    return [cmd];
}

function prepareAaptPackage(project, product, inputs, outputs, input, output, explicitlyDependsOn) {
    var cmds = [];
    var apkOutput = outputs["android.apk"][0];
    var args = commonAaptPackageArgs.apply(this, arguments);
    args.push("-F", apkOutput.filePath + ".unaligned");
    args.push(product.Android.sdk.apkContentsDir);
    var cmd = new Command(product.Android.sdk.aaptFilePath, args);
    cmd.description = "Generating " + apkOutput.filePath;
    cmds.push(cmd);

    if (!product.Android.sdk.useApksigner) {
        args = ["-sigalg", "SHA1withRSA", "-digestalg", "SHA1",
                "-keystore", inputs["android.keystore"][0].filePath,
                "-storepass", "android",
                apkOutput.filePath + ".unaligned",
                "androiddebugkey"];
        cmd = new Command(product.java.jarsignerFilePath, args);
        cmd.description = "Signing " + apkOutput.fileName;
        cmds.push(cmd);
    }

    cmd = new Command(product.Android.sdk.zipalignFilePath,
                      ["-f", "4", apkOutput.filePath + ".unaligned", apkOutput.filePath]);
    cmd.silent = true;
    cmds.push(cmd);

    cmd = new JavaScriptCommand();
    cmd.silent = true;
    cmd.unalignedApk = apkOutput.filePath + ".unaligned";
    cmd.sourceCode = function() { File.remove(unalignedApk); };
    cmds.push(cmd);

    if (product.Android.sdk.useApksigner) {
        // TODO: Implement full signing support, not just using the debug keystore
        args = ["sign",
                "--ks", inputs["android.keystore"][0].filePath,
                "--ks-pass", "pass:android",
                apkOutput.filePath];
        cmd = new Command(product.Android.sdk.apksignerFilePath, args);
        cmd.description = "Signing " + apkOutput.fileName;
        cmds.push(cmd);
    }

    return cmds;
}

function createDebugKeyStoreCommandString(keytoolFilePath, keystoreFilePath) {
    var args = ["-genkey", "-keystore", keystoreFilePath, "-alias", "androiddebugkey",
                "-storepass", "android", "-keypass", "android", "-keyalg", "RSA",
                "-keysize", "2048", "-validity", "10000", "-dname",
                "CN=Android Debug,O=Android,C=US"];
    return Process.shellQuote(keytoolFilePath, args);
}

function gdbserverOrStlDeploymentData(product, inputs, type)
{
    var data = { uniqueInputs: [], outputFilePaths: []};
    var uniqueFilePaths = [];
    var theInputs = inputs[type === "gdbserver" ? "android.gdbserver" : "android.stl"];
    if (!theInputs)
        return data;
    for (var i = 0; i < theInputs.length; ++i) {
        var currentInput = theInputs[i];
        if (uniqueFilePaths.contains(currentInput.filePath))
            continue;
        uniqueFilePaths.push(currentInput.filePath);
        data.uniqueInputs.push(currentInput);
        var outputFileName = type === "gdbserver" ? "libgdbserver.so" : currentInput.fileName;
        data.outputFilePaths.push(FileInfo.joinPaths(product.Android.sdk.apkContentsDir, "lib",
                                                     currentInput.Android.ndk.abi,
                                                     outputFileName));
    }
    return data;
}
