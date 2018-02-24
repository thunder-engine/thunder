/****************************************************************************
**
** Copyright (C) 2015 Jake Petroules.
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

var FileInfo = require("qbs.FileInfo");
var ModUtils = require("qbs.ModUtils");
var Process = require("qbs.Process");

function findTscVersion(compilerFilePath, nodejsPath) {
    var p = new Process();
    try {
        if (nodejsPath)
            p.setEnv("PATH", nodejsPath);
        p.exec(compilerFilePath, ["--version"]);
        var re = /^(?:message TS6029: )?Version (([0-9]+(?:\.[0-9]+){1,3})(?:-(.+?))?)$/m;
        var match = p.readStdOut().trim().match(re);
        if (match !== null)
            return match;
    } finally {
        p.close();
    }
}

function tscArguments(product, inputs) {
    var i;
    var args = [];

    if (ModUtils.moduleProperty(product, "warningLevel") === "pedantic") {
        args.push("--noImplicitAny");
    }

    var targetVersion = ModUtils.moduleProperty(product, "targetVersion");
    if (targetVersion) {
        args.push("--target");
        args.push(targetVersion);
    }

    var moduleLoader = ModUtils.moduleProperty(product, "moduleLoader");
    if (moduleLoader) {
        args.push("--module");
        args.push(moduleLoader);
    }

    if (ModUtils.moduleProperty(product, "stripComments")) {
        args.push("--removeComments");
    }

    if (ModUtils.moduleProperty(product, "generateDeclarations")) {
        args.push("--declaration");
    }

    if (ModUtils.moduleProperty(product, "generateSourceMaps")) {
        args.push("--sourcemap");
    }

    // User-supplied flags
    var flags = ModUtils.moduleProperty(product, "compilerFlags");
    for (i in flags) {
        args.push(flags[i]);
    }

    if (supportsModernFeatures(product)) {
        args.push("--rootDir", product.sourceDirectory);
    }

    args.push("--outDir", product.buildDirectory);

    if (ModUtils.moduleProperty(product, "singleFile")) {
        args.push(outOption(product),
                  FileInfo.joinPaths(product.destinationDirectory, product.targetName) + ".js");
    }

    if (inputs.typescript_declaration) {
        for (i = 0; i < inputs.typescript_declaration.length; ++i) {
            args.push(inputs.typescript_declaration[i].filePath);
        }
    }

    if (inputs.typescript) {
        for (i = 0; i < inputs.typescript.length; ++i) {
            args.push(inputs.typescript[i].filePath);
        }
    }

    if (inputs["typescript.typescript-internal"]) {
        for (i = 0; i < inputs["typescript.typescript-internal"].length; ++i) {
            args.push(inputs["typescript.typescript-internal"][i].filePath);
        }
    }

    return args;
}

function outputArtifacts(product, inputs) {
    if (!supportsModernFeatures(product)) {
        console.warn("Qbs does not properly support TypeScript versions prior to 1.5 due to " +
                     "severe limitations in dependency tracking. This is TypeScript version " +
                     ModUtils.moduleProperty(product, "version") + ". It is strongly recommended " +
                     "that you upgrade TypeScript, or continue at your own risk.");
        return legacyOutputArtifacts(product, inputs);
    }

    var process;
    try {
        process = new Process();
        process.setEnv("NODE_PATH", [
            ModUtils.moduleProperty(product, "toolchainInstallPath"),
            product.moduleProperty("nodejs", "packageManagerRootPath")
        ].join(product.moduleProperty("qbs", "pathListSeparator")));
        process.exec(product.moduleProperty("nodejs", "interpreterFilePath"),
                     [FileInfo.joinPaths(product.buildDirectory,
                                         ".io.qt.qbs.internal.typescript",
                                         "qbs-tsc-scan.js")]
                     .concat(tscArguments(product, inputs)), true);
        var artifacts = JSON.parse(process.readStdOut());

        // Find and tag the "main" output file
        var applicationFile = product.moduleProperty("nodejs", "applicationFile");
        if (applicationFile) {
            var i, appIndex = -1;
            if (product.moduleProperty("typescript", "singleFile")) {
                for (i = 0; i < artifacts.length; ++i) {
                    if (artifacts[i].fileTags.contains("compiled_typescript")) {
                        appIndex = i;
                        break;
                    }
                }
            } else {
                var expected = FileInfo.relativePath(product.sourceDirectory, applicationFile);
                if (!expected.endsWith(".ts"))
                    // tsc doesn't allow this anyways, so it's a perfectly reasonable restriction
                    throw "TypeScript source file name '" + applicationFile +
                            "' does not end with .ts";

                expected = expected.slice(0, -2) + "js";

                for (i = 0; i < artifacts.length; ++i) {
                    if (expected === FileInfo.relativePath(product.buildDirectory,
                                                           artifacts[i].filePath)) {
                        appIndex = i;
                        break;
                    }
                }
            }

            if (appIndex === -1 || !artifacts[appIndex].fileTags.contains("compiled_typescript"))
                throw "nodejs.applicationFile was set, but Qbs couldn't find the compiled " +
                        "JavaScript file corresponding to '" + applicationFile + "'";

            artifacts[appIndex].fileTags = artifacts[appIndex].fileTags.concat(["application_js"]);
        }

        return artifacts;
    } finally {
        if (process)
            process.close();
    }
}

function legacyOutputArtifacts(product, inputs) {
    var artifacts = [];

    if (!inputs.typescript) {
        return artifacts;
    }

    var jsTags = ["js", "compiled_typescript"];
    var filePath = FileInfo.joinPaths(product.destinationDirectory, product.targetName);
    if (product.moduleProperty("typescript", "singleFile")) {
        // We could check
        // if (product.moduleProperty("nodejs", "applicationFile") === inputs.typescript[i].filePath)
        // but since we're compiling to a single file there's no need to state it explicitly
        jsTags.push("application_js");

        artifacts.push({fileTags: jsTags,
                        filePath: FileInfo.joinPaths(
                                      product.moduleProperty("nodejs",
                                                             "compiledIntermediateDir"),
                                      product.targetName + ".js")});

        if (product.moduleProperty("typescript", "generateDeclarations")) {
            artifacts.push({fileTags: ["typescript_declaration"],
                            filePath: filePath + ".d.ts"});
        }

        if (product.moduleProperty("typescript", "generateSourceMaps")) {
            artifacts.push({fileTags: ["source_map"],
                            filePath: filePath + ".js.map"});
        }
    } else {
        for (var i = 0; i < inputs.typescript.length; ++i) {
            jsTags = ["js", "compiled_typescript"];
            if (product.moduleProperty("nodejs", "applicationFile") === inputs.typescript[i].filePath)
                jsTags.push("application_js");

            var intermediatePath = FileInfo.path(FileInfo.relativePath(
                                                     product.sourceDirectory,
                                                     inputs.typescript[i].filePath));

            var baseName = FileInfo.baseName(inputs.typescript[i].fileName);
            filePath = FileInfo.joinPaths(product.destinationDirectory,
                                          intermediatePath,
                                          baseName);

            artifacts.push({fileTags: jsTags,
                            filePath: FileInfo.joinPaths(
                                          product.moduleProperty("nodejs",
                                                                 "compiledIntermediateDir"),
                                          intermediatePath,
                                          baseName + ".js")});

            if (product.moduleProperty("typescript", "generateDeclarations")) {
                artifacts.push({fileTags: ["typescript_declaration"],
                                filePath: filePath + ".d.ts"});
            }

            if (product.moduleProperty("typescript", "generateSourceMaps")) {
                artifacts.push({fileTags: ["source_map"],
                                filePath: filePath + ".js.map"});
            }
        }
    }

    return artifacts;
}

function outOption(product) {
    var compilerVersionMajor = ModUtils.moduleProperty(product, "versionMajor");
    if (compilerVersionMajor === 1) {
        if (ModUtils.moduleProperty(product, "versionMinor") < 6) {
            return "--out";
        }
    }

    return "--outFile";
}

function supportsModernFeatures(product) {
    var compilerVersionMajor = ModUtils.moduleProperty(product, "versionMajor");
    if (compilerVersionMajor === 1) {
        if (ModUtils.moduleProperty(product, "versionMinor") >= 5) {
            return true;
        }
    }

    return compilerVersionMajor > 1;
}
