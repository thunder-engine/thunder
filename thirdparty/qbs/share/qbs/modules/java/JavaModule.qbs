/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

import qbs.FileInfo
import qbs.ModUtils
import qbs.Probes
import qbs.Process
import qbs.TextFile
import qbs.Utilities

import "utils.js" as JavaUtils

Module {
    Probes.JdkProbe {
        id: jdk
        environmentPaths: (jdkPath ? [jdkPath] : []).concat(base)
    }

    Probes.JdkVersionProbe {
        id: jdkVersionProbe
        javac: compilerFilePath
    }

    property stringList additionalClassPaths
    property stringList additionalCompilerFlags
    property stringList additionalJarFlags
    property stringList bootClassPaths
    property string compilerFilePath: FileInfo.joinPaths(jdkPath, "bin", compilerName)
    property string compilerName: "javac"
    property bool enableWarnings: true
    property string interpreterFilePath : FileInfo.joinPaths(jdkPath, "bin", interpreterName)
    property string interpreterName: "java"
    property string jarFilePath: FileInfo.joinPaths(jdkPath, "bin", jarName)
    property string jarName: "jar"
    property string jarsignerFilePath: FileInfo.joinPaths(jdkPath, "bin", jarsignerName)
    property string jarsignerName: "jarsigner"
    property string keytoolFilePath: FileInfo.joinPaths(jdkPath, "bin", keytoolName)
    property string keytoolName: "keytool"

    property bool _tagJniHeaders: true

    property string jdkPath: jdk.path

    version: [compilerVersionMajor, compilerVersionMinor, compilerVersionPatch].join(".")
    property string compilerVersion: jdkVersionProbe.version
                                     ? jdkVersionProbe.version[1] : undefined
    property var compilerVersionParts: compilerVersion ? compilerVersion.split(/[\._]/).map(function(item) { return parseInt(item, 10); }) : []
    property int compilerVersionMajor: compilerVersionParts[0]
    property int compilerVersionMinor: compilerVersionParts[1]
    property int compilerVersionPatch: compilerVersionParts[2]
    property int compilerVersionUpdate: compilerVersionParts[3]

    property string languageVersion
    PropertyOptions {
        name: "languageVersion"
        description: "Java language version to interpret source code as"
    }

    property string runtimeVersion
    PropertyOptions {
        name: "runtimeVersion"
        description: "version of the Java runtime to generate compatible bytecode for"
    }

    property var manifest: {
        return {
            "Manifest-Version": "1.0",
            "Class-Path": manifestClassPath ? manifestClassPath.join(" ") : undefined
        };
    }

    PropertyOptions {
        name: "manifest"
        description: "properties to add to the manifest file when building a JAR"
    }

    PropertyOptions {
        name: "manifestFile"
        description: "Use files tagged \"java.manifest\" instead."
        removalVersion: "1.9"
    }
    property stringList manifestClassPath
    PropertyOptions {
        name: "manifestClassPath"
        description: "entries to add to the manifest's Class-Path when building a JAR"
    }

    property bool warningsAsErrors: false

    property pathList jdkIncludePaths: {
        var paths = [];
        if (isAppleJava) {
            paths.push(FileInfo.joinPaths(qbs.sysroot,
                "/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers"));
        } else {
            paths.push(FileInfo.joinPaths(jdkPath, "include"));

            var hostOS = qbs.hostOS.contains("windows") ? qbs.hostOS.concat(["win32"]) : qbs.hostOS;
            var platforms = ["win32", "darwin", "linux", "bsd", "solaris"];
            for (var i = 0; i < platforms.length; ++i) {
                if (hostOS.contains(platforms[i])) {
                    // Corresponds to JDK_INCLUDE_SUBDIR in the JDK Makefiles
                    paths.push(FileInfo.joinPaths(jdkPath, "include", platforms[i]));
                    break;
                }
            }
        }

        return paths;
    }

    // Internal properties
    property path classFilesDir: FileInfo.joinPaths(product.buildDirectory, "classes")
    property path internalClassFilesDir: FileInfo.joinPaths(product.buildDirectory, ".classes")

    property bool isAppleJava: qbs.hostOS.contains("darwin")
                               && (compilerVersionMajor < 1
                                   || (compilerVersionMajor === 1 && compilerVersionMinor < 7))

    // https://developer.apple.com/library/content/documentation/Java/Conceptual/Java14Development/02-JavaDevTools/JavaDevTools.html
    // tools.jar does not exist. Classes usually located here are instead included in classes.jar.
    // The same is true for rt.jar, although not mentioned in the documentation
    property path classesJarPath: {
        if (isAppleJava)
            return FileInfo.joinPaths(jdkPath, "bundle", "Classes", "classes.jar");
    }

    property path runtimeJarPath: {
        if (compilerVersionMajor >= 9)
            return undefined;
        if (classesJarPath)
            return classesJarPath;
        return FileInfo.joinPaths(jdkPath, "jre", "lib", "rt.jar");
    }

    property path toolsJarPath: {
        if (compilerVersionMajor >= 9)
            return undefined;
        if (classesJarPath)
            return classesJarPath;
        return FileInfo.joinPaths(jdkPath, "lib", "tools.jar");
    }

    validate: {
        var validator = new ModUtils.PropertyValidator("java");
        validator.setRequiredProperty("jdkPath", jdkPath);
        validator.setRequiredProperty("compilerVersion", compilerVersion);
        validator.setRequiredProperty("compilerVersionParts", compilerVersionParts);
        validator.setRequiredProperty("compilerVersionMajor", compilerVersionMajor);
        validator.setRequiredProperty("compilerVersionMinor", compilerVersionMinor);
        if (Utilities.versionCompare(version, "9") < 0)
            validator.setRequiredProperty("compilerVersionUpdate", compilerVersionUpdate);
        validator.addVersionValidator("compilerVersion", compilerVersion
                                      ? compilerVersion.replace("_", ".") : undefined, 3, 4);
        validator.addRangeValidator("compilerVersionMajor", compilerVersionMajor, 1);
        validator.addRangeValidator("compilerVersionMinor", compilerVersionMinor, 0);
        validator.addRangeValidator("compilerVersionPatch", compilerVersionPatch, 0);
        if (Utilities.versionCompare(version, "9") < 0)
            validator.addRangeValidator("compilerVersionUpdate", compilerVersionUpdate, 0);
        validator.validate();
    }

    FileTagger {
        patterns: "*.java"
        fileTags: ["java.java"]
    }

    FileTagger {
        patterns: ["*.mf"]
        fileTags: ["java.manifest"]
    }

    Group {
        name: "io.qt.qbs.internal.java-helper"
        files: {
            return JavaUtils.helperFullyQualifiedNames("java").map(function(name) {
                return FileInfo.joinPaths(path, name + ".java");
            });
        }

        fileTags: ["java.java-internal"]
    }

    Rule {
        multiplex: true
        inputs: ["java.java-internal"]

        outputFileTags: ["java.class-internal"]
        outputArtifacts: {
            return JavaUtils.helperOutputArtifacts(product);
        }

        prepare: {
            var cmd = new Command(ModUtils.moduleProperty(product, "compilerFilePath"),
                                  JavaUtils.javacArguments(product, inputs,
                                                           JavaUtils.helperOverrideArgs(product,
                                                                                        "javac")));
            cmd.ignoreDryRun = true;
            cmd.silent = true;
            return [cmd];
        }
    }

    Rule {
        multiplex: true
        inputs: ["java.java"]
        inputsFromDependencies: ["java.jar"]
        explicitlyDependsOn: ["java.class-internal"]

        outputFileTags: ["java.class"].concat(_tagJniHeaders ? ["hpp"] : []) // Annotations can produce additional java source files. Ignored for now.
        outputArtifacts: {
            var artifacts = JavaUtils.outputArtifacts(product, inputs);
            if (!product.java._tagJniHeaders) {
                for (var i = 0; i < artifacts.length; ++i) {
                    var a = artifacts[i];
                    if (Array.isArray(a.fileTags))
                        a.fileTags = a.fileTags.filter(function(tag) { return tag != "hpp"; });
                }
            }
            return artifacts;
        }
        prepare: {
            var cmd = new Command(ModUtils.moduleProperty(product, "compilerFilePath"),
                                  JavaUtils.javacArguments(product, inputs));
            cmd.description = "Compiling Java sources";
            cmd.highlight = "compiler";
            return [cmd];
        }
    }

    Rule {
        inputs: ["java.class", "java.manifest"]
        multiplex: true

        Artifact {
            fileTags: ["java.jar"]
            filePath: FileInfo.joinPaths(product.destinationDirectory, product.targetName + ".jar")
        }

        prepare: {
            var i, key;
            var flags = "cf";
            var args = [output.filePath];

            var aggregateManifest = {};
            var manifestFiles = (inputs["java.manifest"] || []).map(function (a) { return a.filePath; });
            manifestFiles.forEach(function (manifestFile) {
                var mf = JavaUtils.manifestContents(manifestFile);
                for (key in mf) {
                    if (mf.hasOwnProperty(key)) {
                        var oldValue = aggregateManifest[key];
                        var newValue = mf[key];
                        if (oldValue !== undefined && oldValue !== newValue) {
                            throw new Error("Conflicting values '"
                                            + oldValue + "' and '"
                                            + newValue + "' for manifest file key '" + key + "'");
                        }

                        aggregateManifest[key] = newValue;
                    }
                }
            });

            // Add local key-value pairs (overrides equivalent keys specified in the file if
            // one was given)
            var manifest = product.java.manifest;
            for (key in manifest) {
                if (manifest.hasOwnProperty(key))
                    aggregateManifest[key] = manifest[key];
            }

            for (key in aggregateManifest) {
                if (aggregateManifest.hasOwnProperty(key) && aggregateManifest[key] === undefined)
                    delete aggregateManifest[key];
            }

            // Use default manifest unless we actually have properties to set
            var needsManifestFile = manifestFiles.length > 0
                    || aggregateManifest !== {"Manifest-Version": "1.0"};

            var manifestFile = FileInfo.joinPaths(product.buildDirectory, "manifest.mf");

            var mf;
            try {
                mf = new TextFile(manifestFile, TextFile.WriteOnly);

                // Ensure that manifest version comes first
                mf.write("Manifest-Version: " + (aggregateManifest["Manifest-Version"] || "1.0") + "\n");
                delete aggregateManifest["Manifest-Version"];

                for (key in aggregateManifest)
                    mf.write(key + ": " + aggregateManifest[key] + "\n");

                mf.write("\n");
            } finally {
                if (mf) {
                    mf.close();
                }
            }

            if (needsManifestFile) {
                flags += "m";
                args.push(manifestFile);
            }

            var entryPoint = ModUtils.moduleProperty(product, "entryPoint");
            var entryPoint = product.entryPoint;
            if (entryPoint) {
                flags += "e";
                args.push(entryPoint);
            }

            args.unshift(flags);

            var otherFlags = ModUtils.moduleProperty(product, "additionalJarFlags");
            if (otherFlags)
                args = args.concat(otherFlags);

            for (i in inputs["java.class"])
                args.push(FileInfo.relativePath(ModUtils.moduleProperty(product, "classFilesDir"),
                                                inputs["java.class"][i].filePath));
            var cmd = new Command(ModUtils.moduleProperty(product, "jarFilePath"), args);
            cmd.workingDirectory = ModUtils.moduleProperty(product, "classFilesDir");
            cmd.description = "building " + FileInfo.fileName(output.fileName);
            cmd.highlight = "linker";
            return cmd;
        }
    }
}
