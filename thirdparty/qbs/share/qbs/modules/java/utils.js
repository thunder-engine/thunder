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
var ModUtils = require("qbs.ModUtils");
var Process = require("qbs.Process");
var TextFile = require("qbs.TextFile");
var Utilities = require("qbs.Utilities");
var WindowsUtils = require("qbs.WindowsUtils");

function is64bitProcess() {
    var y = jdkRootRegistryKey(true);
    var n = jdkRootRegistryKey(false);
    y = Utilities.getNativeSetting(y + "\\" + Utilities.getNativeSetting(y, "CurrentVersion"), "JavaHome");
    n = Utilities.getNativeSetting(n + "\\" + Utilities.getNativeSetting(n, "CurrentVersion"), "JavaHome");
    return y !== n;
}

function useWow64Key(arch) {
    var wow64 = false;
    switch (arch) {
    case "x86_64":
    case "ia64":
        // QTBUG-3845
        if (!is64bitProcess())
            return undefined;
        break;
    case "x86":
    case "armv7":
        wow64 = is64bitProcess();
        break;
    }
    return wow64;
}

function jdkRootRegistryKey(wow64) {
    // If an architecture is specified, search the appropriate key for that architecture,
    // on this version of Windows (i.e. WOW64 or not) if compatible,
    // otherwise get both keys since any JDK will be usable
    if (wow64 === undefined)
        return undefined;
    return FileInfo.toWindowsSeparators(FileInfo.joinPaths("HKEY_LOCAL_MACHINE", "SOFTWARE",
                                                           (wow64 ? "Wow6432Node" : undefined),
                                                           "JavaSoft", "Java Development Kit"));
}

function findJdkPath(hostOS, arch, environmentPaths, searchPaths) {
    var i;
    for (var key in environmentPaths) {
        if (environmentPaths[key]) {
            return environmentPaths[key];
        }
    }

    if (hostOS.contains("windows")) {
        var rootKey = jdkRootRegistryKey(useWow64Key(arch));
        if (rootKey) {
            var current = Utilities.getNativeSetting(rootKey, "CurrentVersion"); // 1.8 etc.
            if (current) {
                var home = Utilities.getNativeSetting([rootKey, current].join("\\"), "JavaHome");
                if (home) {
                    return home;
                }
            }
        }

        return undefined;
    }

    if (hostOS.contains("macos")) {
        var p = new Process();
        try {
            // We filter by architecture here so that we'll get a compatible JVM for JNI use.
            var args = [];
            if (arch) {
                // Hardcoding apple/macosx/macho here is fine because we know we're on macOS
                args.push("--arch",
                          Utilities.canonicalTargetArchitecture(arch, undefined,
                                                                "apple", "macosx", "macho"));
            }

            // --failfast doesn't print the default JVM if nothing matches the filter(s).
            var status = p.exec("/usr/libexec/java_home", args.concat(["--failfast"]));
            return status === 0 ? p.readStdOut().trim() : undefined;
        } finally {
            p.close();
        }
    }

    if (hostOS.contains("unix")) {
        var requiredTools = ["javac", "java", "jar"];
        for (i = 0; i < searchPaths.length; ++i) {
            function fullToolPath(tool) {
                return FileInfo.joinPaths(searchPaths[i], "bin", tool);
            }

            if (requiredTools.map(function(p) { return fullToolPath(p); })
                    .every(function(p) { return File.exists(p); })) {
                return searchPaths[i];
            }
        }

        return undefined;
    }
}

function findJdkVersion(compilerFilePath) {
    var p = new Process();
    try {
        p.exec(compilerFilePath, ["-version"]);
        var re = /^javac (([0-9]+(?:\.[0-9]+){2,2})(_([0-9]+))?)$/m;
        var match = p.readStdErr().trim().match(re);
        if (!match)
            match = p.readStdOut().trim().match(re);
        if (match !== null)
            return match;
    } finally {
        p.close();
    }
}

function supportsGeneratedNativeHeaderFiles(product) {
    var compilerVersionMajor = ModUtils.moduleProperty(product, "compilerVersionMajor");
    if (compilerVersionMajor === 1) {
        if (ModUtils.moduleProperty(product, "compilerVersionMinor") >= 8) {
            return true;
        }
    }

    return compilerVersionMajor > 1;
}

function javacArguments(product, inputs, overrides) {
    function getModuleProperty(product, propertyName, overrides) {
        if (overrides && overrides[propertyName])
            return overrides[propertyName];
        return ModUtils.moduleProperty(product, propertyName);
    }

    function getModuleProperties(product, propertyName, overrides) {
        if (overrides && overrides[propertyName])
            return overrides[propertyName];
        return ModUtils.moduleProperty(product, propertyName);
    }

    var i;
    var outputDir = getModuleProperty(product, "classFilesDir", overrides);
    var classPaths = [outputDir];
    var additionalClassPaths = getModuleProperties(product, "additionalClassPaths", overrides);
    if (additionalClassPaths)
        classPaths = classPaths.concat(additionalClassPaths);
    for (i in inputs["java.jar"])
        classPaths.push(inputs["java.jar"][i].filePath);
    var debugArg = product.moduleProperty("qbs", "buildVariant") === "debug"
            ? "-g" : "-g:none";
    var pathListSeparator = product.moduleProperty("qbs", "pathListSeparator");
    var args = [
            "-classpath", classPaths.join(pathListSeparator),
            "-s", product.buildDirectory,
            debugArg, "-d", outputDir
        ];
    if (supportsGeneratedNativeHeaderFiles(product))
        args.push("-h", product.buildDirectory);
    var runtimeVersion = getModuleProperty(product, "runtimeVersion", overrides);
    if (runtimeVersion)
        args.push("-target", runtimeVersion);
    var languageVersion = getModuleProperty(product, "languageVersion", overrides);
    if (languageVersion)
        args.push("-source", languageVersion);
    var bootClassPaths = getModuleProperties(product, "bootClassPaths", overrides);
    if (bootClassPaths && bootClassPaths.length > 0
            && (!runtimeVersion || Utilities.versionCompare(runtimeVersion, "9") < 0)) {
        args.push("-bootclasspath", bootClassPaths.join(pathListSeparator));
    }
    if (!getModuleProperty(product, "enableWarnings", overrides))
        args.push("-nowarn");
    if (getModuleProperty(product, "warningsAsErrors", overrides))
        args.push("-Werror");
    var otherFlags = getModuleProperties(product, "additionalCompilerFlags", overrides);
    if (otherFlags)
        args = args.concat(otherFlags);
    for (i in inputs["java.java"])
        args.push(inputs["java.java"][i].filePath);
    for (i in inputs["java.java-internal"])
        args.push(inputs["java.java-internal"][i].filePath);
    return args;
}

/**
  * Returns a list of fully qualified Java class names for the compiler helper tool.
  *
  * @param type @c java to return names of sources, @c to return names of compiled classes
  */
function helperFullyQualifiedNames(type) {
    var names = [
        "io/qt/qbs/Artifact",
        "io/qt/qbs/ArtifactListJsonWriter",
        "io/qt/qbs/ArtifactListWriter",
        "io/qt/qbs/tools/JavaCompilerScannerTool",
        "io/qt/qbs/tools/utils/JavaCompilerOptions",
        "io/qt/qbs/tools/utils/JavaCompilerScanner",
        "io/qt/qbs/tools/utils/JavaCompilerScanner$1",
        "io/qt/qbs/tools/utils/NullFileObject",
        "io/qt/qbs/tools/utils/NullFileObject$1",
        "io/qt/qbs/tools/utils/NullFileObject$2",
    ];
    if (type === "java") {
        return names.filter(function (name) {
            return !name.contains("$");
        });
    } else if (type === "class") {
        return names;
    }
}

function helperOutputArtifacts(product) {
    File.makePath(ModUtils.moduleProperty(product, "internalClassFilesDir"));
    return helperFullyQualifiedNames("class").map(function (name) {
        return {
            filePath: FileInfo.joinPaths(ModUtils.moduleProperty(product, "internalClassFilesDir"),
                                         name + ".class"),
            fileTags: ["java.class-internal"]
        };
    });
}

function helperOverrideArgs(product, tool) {
    var overrides = {};
    if (tool === "javac") {
        // Build the helper tool with the same source and target version as the JDK it's being
        // compiled with. Both are irrelevant here since the resulting tool will only be run
        // with the same JDK as it was built with, and we know in advance the source is
        // compatible with all Java language versions from 1.6 and above.
        var jdkVersionArray = [product.java.compilerVersionMajor];
        if (product.java.compilerVersionMajor < 9)
            jdkVersionArray.push(product.java.compilerVersionMinor);
        var jdkVersion = jdkVersionArray.join(".");
        overrides["languageVersion"] = jdkVersion;
        overrides["runtimeVersion"] = jdkVersion;

        // Build the helper tool's class files separately from the actual product's class files
        overrides["classFilesDir"] = ModUtils.moduleProperty(product, "internalClassFilesDir");

        // Add tools.jar to the classpath as required for the tree scanner API
        var toolsJarPath = ModUtils.moduleProperty(product, "toolsJarPath");
        if (toolsJarPath)
            overrides["additionalClassPaths"] = [toolsJarPath].concat(
                        ModUtils.moduleProperty(product, "additionalClassPaths"));
    }

    // Inject the current JDK's runtime classes into the boot class path when building/running the
    // dependency scanner. This is normally not necessary but is important for Android platforms
    // where android.jar is the only JAR on the boot classpath and JSR 199 is unavailable.
    var rtJarPath = product.java.runtimeJarPath;
    overrides["bootClassPaths"] = (rtJarPath ? [rtJarPath] : []).concat(
                ModUtils.moduleProperty(product, "bootClassPaths"));
    return overrides;
}

function outputArtifacts(product, inputs) {
    // Handle the case where a product depends on Java but has no Java sources
    if (!inputs["java.java"] || inputs["java.java"].length === 0)
        return [];

    // We need to ensure that the output directory is created first, because the Java compiler
    // internally checks that it is present before performing any actions
    File.makePath(ModUtils.moduleProperty(product, "classFilesDir"));

    var process;
    try {
        process = new Process();
        process.setWorkingDirectory(
                    FileInfo.joinPaths(ModUtils.moduleProperty(product, "internalClassFilesDir")));

        var sep = product.moduleProperty("qbs", "pathListSeparator");
        var toolsJarPath = ModUtils.moduleProperty(product, "toolsJarPath");
        var javaArgs = [
            "-classpath", process.workingDirectory() + (toolsJarPath ? (sep + toolsJarPath) : ""),
            "io/qt/qbs/tools/JavaCompilerScannerTool",
        ];
        process.exec(ModUtils.moduleProperty(product, "interpreterFilePath"), javaArgs
                     .concat(javacArguments(product, inputs, helperOverrideArgs(product))), true);
        var out = JSON.parse(process.readStdOut());
        console.error(process.readStdErr());
        return out;
    } finally {
        if (process)
            process.close();
    }
}

function manifestContents(filePath) {
    if (filePath === undefined)
        return undefined;

    var contents, file;
    try {
        file = new TextFile(filePath);
        contents = file.readAll();
    } finally {
        if (file) {
            file.close();
        }
    }

    if (contents) {
        var dict = {};
        var lines = contents.split(/\r?\n/g).filter(function (line) { return line.length > 0; });
        for (var i in lines) {
            var kv = lines[i].split(":");
            if (kv.length !== 2)
                throw new Error("Syntax error in manifest file '"
                                + filePath + "'; found \"" + lines[i] + "\" on line "
                                + parseInt(i, 10) + "; expected format \"Key: Value\"");
            dict[kv[0].trim()] = kv[1].trim();
        }

        return dict;
    }
}
