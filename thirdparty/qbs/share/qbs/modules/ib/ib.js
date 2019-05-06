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

var BundleTools = require("qbs.BundleTools");
var DarwinTools = require("qbs.DarwinTools");
var File = require("qbs.File");
var FileInfo = require("qbs.FileInfo");
var ModUtils = require("qbs.ModUtils");
var Process = require("qbs.Process");
var PropertyList = require("qbs.PropertyList");

function artifactsFromInputs(inputs) {
    var artifacts = [];
    for (var tag in inputs) {
        artifacts = artifacts.concat(inputs[tag]);
    }
    return artifacts;
}

function tiffutilScalesMap(inputs) {
    return artifactsFromInputs(inputs).map(function (a) {
        var m = a.filePath.match(/^(.+?)(@(\d+)x)?(\..+?)$/);
        var basePath = m[1];
        var scale = m[2] || "";
        var nscale = m[3];
        var extension = m[4];
        if (scale && scale < 1)
            throw new Error("Invalid scale '" + nscale + "' for image '" + a.filePath + "'");
        return {
            basePath: basePath,
            extension: extension,
            scale: scale
        };
    }).reduce(function (previous, current) {
        previous[current["basePath"]] = (previous[current["basePath"]] || []).concat([{
            extension: current["extension"],
            scale: current["scale"]
        }]);
        return previous;
    }, {});
}

function tiffutilOutputFilePath(product, basePath) {
    return FileInfo.joinPaths(product.destinationDirectory,
                              "hidpi-images",
                              FileInfo.relativePath(product.sourceDirectory, basePath) +
                              product.ib.tiffSuffix);
}

function tiffutilArtifacts(product, inputs) {
    var artifacts = [];
    var map = tiffutilScalesMap(inputs);
    for (var key in map) {
        artifacts.push({
            filePath: tiffutilOutputFilePath(product, key),
            fileTags: ["tiff"]
        });
    }
    return artifacts;
}

function prepareTiffutil(project, product, inputs, outputs, input, output) {
    var cmds = [];
    var map = tiffutilScalesMap(inputs);
    for (var key in map) {
        var args = ["-cat" + (product.ib.combineHidpiImages ? "hidpicheck" : "")];
        var count = 0;
        map[key].forEach(function (obj) {
            args.push(key + obj["scale"] + obj["extension"]);
            ++count;
        });
        args.push("-out", tiffutilOutputFilePath(product, key));
        var cmd = new Command(product.ib.tiffutilPath, args);
        cmd.description = "creating " + output.fileName;
        cmd.count = count;
        cmd.outputFilePath = output.filePath;
        cmd.stderrFilterFunction = function (output) {
            return output.replace(count + " images written to " + outputFilePath + ".", "");
        };
        cmds.push(cmd);
    }
    return cmds;
}

function ibtooldArguments(product, inputs, input, outputs, overrideOutput) {
    var i;
    var args = [];
    var allInputs = artifactsFromInputs(inputs);

    var outputFormat = ModUtils.moduleProperty(product, "outputFormat");
    if (outputFormat) {
        if (!["binary1", "xml1", "human-readable-text"].contains(outputFormat))
            throw("Invalid ibtoold output format: " + outputFormat + ". " +
                  "Must be in [binary1, xml1, human-readable-text].");

        args.push("--output-format", outputFormat);
    }

    var debugFlags = ["warnings", "errors", "notices"];
    for (var j in debugFlags) {
        var flag = debugFlags[j];
        if (ModUtils.modulePropertyFromArtifacts(product, allInputs, product.moduleName, flag)) {
            args.push("--" + flag);
        }
    }

    if (inputs.assetcatalog) {
        args.push("--platform", DarwinTools.applePlatformName(
                      product.moduleProperty("qbs", "targetOS"),
                      product.moduleProperty("xcode", "platformType")));

        var appIconName = ModUtils.modulePropertyFromArtifacts(product, inputs.assetcatalog, product.moduleName, "appIconName");
        if (appIconName)
            args.push("--app-icon", appIconName);

        var launchImageName = ModUtils.modulePropertyFromArtifacts(product, inputs.assetcatalog, product.moduleName, "launchImageName");
        if (launchImageName)
            args.push("--launch-image", launchImageName);

        // Undocumented but used by Xcode (only for iOS?), probably runs pngcrush or equivalent
        if (ModUtils.modulePropertyFromArtifacts(product, inputs.assetcatalog, product.moduleName, "compressPngs"))
            args.push("--compress-pngs");
    } else {
        var sysroot = product.moduleProperty("qbs", "sysroot");
        if (sysroot)
            args.push("--sdk", sysroot);

        args.push("--flatten", ModUtils.modulePropertyFromArtifacts(product, allInputs, product.moduleName, "flatten") ? 'YES' : 'NO');

        // --module and --auto-activate-custom-fonts were introduced in Xcode 6.0
        if (ModUtils.moduleProperty(product, "ibtoolVersionMajor") >= 6) {
            var module = ModUtils.moduleProperty(product, "module");
            if (module)
                args.push("--module", module);

            if (ModUtils.modulePropertyFromArtifacts(product, allInputs, product.moduleName, "autoActivateCustomFonts"))
                args.push("--auto-activate-custom-fonts");
        }
    }

    // --minimum-deployment-target was introduced in Xcode 5.0
    var minimumDarwinVersion = product.moduleProperty("cpp", "minimumDarwinVersion");
    if (minimumDarwinVersion && ModUtils.moduleProperty(product, "ibtoolVersionMajor") >= 5)
        args.push("--minimum-deployment-target", minimumDarwinVersion);

    // --target-device and -output-partial-info-plist were introduced in Xcode 6.0 for ibtool
    if (ModUtils.moduleProperty(product, "ibtoolVersionMajor") >= 6 || inputs.assetcatalog) {
        args.push("--output-partial-info-plist", (outputs && outputs.partial_infoplist)
                  ? outputs.partial_infoplist[0].filePath
                  : "/dev/null");

        // For iOS, we'd normally only output the devices specified in TARGETED_DEVICE_FAMILY
        // We can't get this info from Info.plist keys due to dependency order, so use the qbs prop
        var targetDevices = ModUtils.moduleProperty(product, "targetDevices");
        for (i in targetDevices) {
            args.push("--target-device", targetDevices[i]);
        }
    }

    args = args.concat(ModUtils.modulePropertiesFromArtifacts(product, allInputs,
                                                              product.moduleName, "flags"));

    if (overrideOutput) {
        args.push("--compile", overrideOutput);
    } else {
        if (outputs.compiled_assetcatalog)
            args.push("--compile", product.buildDirectory + "/actool.dir");
        else // compiled_ibdoc
            args.push("--compile", product.buildDirectory + "/ibtool.dir/"
                      + ibtoolCompiledDirSuffix(product, input));
    }

    for (i in allInputs)
        args.push(allInputs[i].filePath);

    return args;
}

function ibtoolFileTaggers(fileTags) {
    var ext;
    if (fileTags.contains("nib") && !fileTags.contains("storyboard"))
        ext = "nib";
    if (fileTags.contains("storyboard") && !fileTags.contains("nib"))
        ext = "storyboard";

    if (!ext)
        throw "unknown ibtool input file tags: " + fileTags;

    var t = ["bundle.input", "compiled_ibdoc"];
    return {
        ".nib": t.concat(["compiled_" + ext + (ext !== "nib" ? "_nib" : "")]),
        ".plist": t.concat(["compiled_" + ext + "_infoplist"]),
        ".storyboard": t.concat(["compiled_" + ext])
    };
}

function ibtoolCompiledDirSuffix(product, input) {
    var suffix = input.completeBaseName;
    if (input.fileTags.contains("nib"))
        suffix += ModUtils.moduleProperty(product, "compiledNibSuffix");
    else if (input.fileTags.contains("storyboard"))
        suffix += ModUtils.moduleProperty(product, "compiledStoryboardSuffix");
    return suffix;
}

function ibtoolOutputArtifacts(product, inputs, input) {
    var suffix = ibtoolCompiledDirSuffix(product, input);
    var tracker = new ModUtils.BlackboxOutputArtifactTracker();
    tracker.hostOS = product.moduleProperty("qbs", "hostOS");
    tracker.shellPath = product.moduleProperty("qbs", "shellPath");
    tracker.fileTaggers = ibtoolFileTaggers(input.fileTags);
    tracker.command = ModUtils.moduleProperty(product, "ibtoolPath");
    tracker.commandArgsFunction = function (outputDirectory) {
        // Last --output-format argument overrides any previous ones
        // Append the name of the base output since it can be either a file or a directory
        // in the case of XIB compilations
        return ibtooldArguments(product, inputs, input,
                                undefined, FileInfo.joinPaths(outputDirectory, suffix))
            .concat(["--output-format", "xml1"]);
    };

    var ibtoolBuildDirectory = product.buildDirectory + "/ibtool.dir";
    var main = BundleTools.destinationDirectoryForResource(product, input);

    var artifacts = tracker.artifacts(ibtoolBuildDirectory);

    if (product.moduleProperty("ib", "ibtoolVersionMajor") >= 6) {
        var prefix = input.fileTags.contains("storyboard") ? "SB" : "";
        var path = FileInfo.joinPaths(product.destinationDirectory, input.completeBaseName +
                                      "-" + prefix + "PartialInfo.plist");
        artifacts.push({ filePath: path, fileTags: ["partial_infoplist"] });
    }

    // Let the output artifacts known the "main" output
    // This can be either a file or directory so the artifact might already exist in the output list
    for (var i = 0; i < artifacts.length; ++i) {
        if (artifacts[i].fileTags.contains("compiled_ibdoc"))
            artifacts[i].bundle = {
                _bundleFilePath: artifacts[i].filePath.replace(ibtoolBuildDirectory, main)
            };
    }

    return artifacts;
}

function actoolOutputArtifacts(product, inputs) {
    // actool has no --dry-run option (rdar://21786925),
    // so compile to a fake temporary directory in order to extract the list of output files
    var tracker = new ModUtils.BlackboxOutputArtifactTracker();
    tracker.hostOS = product.moduleProperty("qbs", "hostOS");
    tracker.shellPath = product.moduleProperty("qbs", "shellPath");
    tracker.command = ModUtils.moduleProperty(product, "actoolPath");
    tracker.commandArgsFunction = function (outputDirectory) {
        // Last --output-format argument overrides any previous ones
        return ibtooldArguments(product, inputs, undefined,
                                undefined, outputDirectory).concat(["--output-format", "xml1"]);
    };
    tracker.processStdOutFunction = parseActoolOutput;
    var artifacts = tracker.artifacts(product.buildDirectory + "/actool.dir");

    // Newer versions of actool don't generate *anything* if there's no input;
    // in that case a partial Info.plist would not have been generated either
    if (artifacts && artifacts.length > 0) {
        artifacts.push({
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         "assetcatalog_generated_info.plist"),
            fileTags: ["partial_infoplist"]
        });
    }

    for (var i = 0; i < artifacts.length; ++i) {
        if (artifacts[i].fileTags.contains("compiled_assetcatalog")) {
            artifacts[i].bundle = {
                _bundleFilePath: artifacts[i].filePath.replace(
                    product.buildDirectory + "/actool.dir",
                    BundleTools.destinationDirectoryForResource(product, inputs.assetcatalog[0]))
            };
        }
    }

    return artifacts;
}

function parseActoolOutput(output) {
    var propertyList = new PropertyList();
    try {
        propertyList.readFromString(output);

        var plist = propertyList.toObject();
        if (plist)
            plist = plist["com.apple.actool.compilation-results"];
        if (plist) {
            var artifacts = [];
            files = plist["output-files"];
            for (var i in files) {
                if (files[i] === "/dev/null")
                    continue;
                var tags = files[i].endsWith(".plist")
                        ? ["partial_infoplist"]
                        : ["bundle.input", "compiled_assetcatalog"];
                artifacts.push({
                    // Even though we pass in a canonical base dir, the paths in the XML File
                    // are non-canonical. See QBS-1417.
                    filePath: FileInfo.canonicalPath(files[i]),
                    fileTags: tags
                });
            }

            return artifacts;
        }
    } finally {
        propertyList.clear();
    }
}

function ibtoolVersion(ibtool) {
    var process;
    var version;
    try {
        process = new Process();
        if (process.exec(ibtool, ["--version", "--output-format", "xml1"], true) !== 0)
            console.error(process.readStdErr());

        var propertyList = new PropertyList();
        try {
            propertyList.readFromString(process.readStdOut());

            var plist = propertyList.toObject();
            if (plist)
                plist = plist["com.apple.ibtool.version"];
            if (plist)
                version = plist["short-bundle-version"];
        } finally {
            propertyList.clear();
        }
    } finally {
        process.close();
    }
    return version;
}
