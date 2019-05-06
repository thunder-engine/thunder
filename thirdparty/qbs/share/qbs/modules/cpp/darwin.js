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

var File = require("qbs.File");
var FileInfo = require("qbs.FileInfo");
var Gcc = require("./gcc.js");
var ModUtils = require("qbs.ModUtils");
var PathTools = require("qbs.PathTools");

function lipoOutputArtifacts(product, inputs, fileTag, debugSuffix) {
    var buildVariants = [];
    for (var i = 0; i < inputs[fileTag].length; ++i) {
        var variant = inputs[fileTag][i].qbs.buildVariant;
        var suffix = inputs[fileTag][i].cpp.variantSuffix;
        if (!buildVariants.some(function (x) { return x.name === variant; }))
            buildVariants.push({ name: variant, suffix: suffix });
    }

    var list = [];

    if (fileTag === "dynamiclibrary") {
        Array.prototype.push.apply(list, buildVariants.map(function (variant) {
            return {
                filePath: product.destinationDirectory + "/.sosymbols/"
                    + PathTools.dynamicLibraryFilePath(product, variant.suffix),
                fileTags: ["dynamiclibrary_symbols"],
                qbs: { buildVariant: variant.name },
                cpp: { variantSuffix: variant.suffix },
                alwaysUpdated: false
            };
        }));
    }

    // Bundles should have a "normal" variant. In the case of frameworks, they cannot normally be
    // linked to without a default variant unless a variant is specifically chosen at link time
    // by passing the full path to the shared library executable instead of the -framework switch.
    // Technically this doesn't affect qbs since qbs always uses full paths for internal
    // dependencies but the "normal" variant is always the one that is linked to, since the
    // alternative variants should only be chosen at runtime using the DYLD_IMAGE_SUFFIX variable.
    // So for frameworks we'll create a symlink to the "default" variant as chosen by the user
    // (we cannot do this automatically since the user must tell us which variant should be
    // preferred, if there are multiple alternative variants). Applications are fine without a
    // symlink but still need an explicitly chosen variant to set as the CFBundleExecutable so that
    // Finder/LaunchServices can launch it normally but for simplicity we'll just use the symlink
    // approach for all bundle types.
    var defaultVariant;
    if (!buildVariants.some(function (x) { return x.name === "release"; })
            && product.multiplexByQbsProperties.contains("buildVariants")
            && product.qbs.buildVariants && product.qbs.buildVariants.length > 1) {
        var defaultBuildVariant = product.qbs.defaultBuildVariant;
        buildVariants.map(function (variant) {
            if (variant.name === defaultBuildVariant)
                defaultVariant = variant;
        });
        if (!defaultVariant) {
            throw new Error("qbs.defaultBuildVariant is '" + defaultBuildVariant + "', but this " +
                            "variant is not in the qbs.buildVariants list (" +
                            product.qbs.buildVariants.join(", ") + ")");
        }

        buildVariants.push({
            name: "release",
            suffix: "",
            isSymLink: true
        });
    }

    Array.prototype.push.apply(list, buildVariants.map(function (variant) {
        var tags = ["bundle.input"];
        if (variant.isSymLink)
            tags.push("bundle.variant_symlink");
        else
            tags.push(fileTag, "primary");

        return {
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         PathTools.linkerOutputFilePath(fileTag, product,
                                                                        variant.suffix)),
            fileTags: tags,
            qbs: {
                buildVariant: variant.name,
                _buildVariantFileName: variant.isSymLink && defaultVariant
                                       ? FileInfo.fileName(PathTools.linkerOutputFilePath(
                                                               fileTag, product,
                                                               defaultVariant.suffix))
                                       : undefined
            },
            bundle: {
                _bundleFilePath: product.destinationDirectory + "/"
                                 + PathTools.bundleExecutableFilePath(product, variant.suffix)
            },
            cpp: {
                variantSuffix: variant.suffix
            }
        };
    }));
    if (debugSuffix)
        Array.prototype.push.apply(list, Gcc.debugInfoArtifacts(product, buildVariants,
                                                                debugSuffix));
    return list;
}

function prepareLipo(project, product, inputs, outputs, input, output) {
    var cmd;
    var commands = [];
    for (var p in inputs)
        inputs[p] = inputs[p].filter(function(inp) { return inp.product.name === product.name; });
    var allInputs = [].concat.apply([], Object.keys(inputs).map(function (tag) {
        return ["application", "dynamiclibrary", "staticlibrary", "loadablemodule"].contains(tag)
                ? inputs[tag] : [];
    }));

    (outputs["bundle.variant_symlink"] || []).map(function (symlink) {
        cmd = new Command("ln", ["-sfn", symlink.qbs._buildVariantFileName, symlink.filePath]);
        cmd.silent = true;
        commands.push(cmd);
    });

    for (var i = 0; i < outputs.primary.length; ++i) {
        var vInputs = allInputs.filter(function (f) {
            return f.qbs.buildVariant === outputs.primary[i].qbs.buildVariant
        }).map(function (f) {
            return f.filePath
        });

        if (vInputs.length > 1 || product.cpp.alwaysUseLipo) {
            cmd = new Command(ModUtils.moduleProperty(product, "lipoPath"),
                              ["-create", "-output", outputs.primary[i].filePath].concat(vInputs));
            cmd.description = "lipo " + outputs.primary[i].fileName;
            cmd.highlight = "linker";
        } else {
            cmd = new JavaScriptCommand();
            cmd.src = vInputs[0];
            cmd.dst = outputs.primary[i].filePath;
            cmd.sourceCode = function () {
                File.copy(src, dst);
            };
            cmd.silent = true;
        }

        commands.push(cmd);
    }

    var debugInfo = outputs.debuginfo_app || outputs.debuginfo_dll
            || outputs.debuginfo_loadablemodule;
    if (debugInfo) {
        var dsymPath = debugInfo[0].filePath;
        if (outputs.debuginfo_bundle && outputs.debuginfo_bundle[0])
            dsymPath = outputs.debuginfo_bundle[0].filePath;
        var flags = ModUtils.moduleProperty(product, "dsymutilFlags") || [];
        cmd = new Command(ModUtils.moduleProperty(product, "dsymutilPath"), flags.concat([
            "-o", dsymPath
        ]).concat(outputs.primary.map(function (f) { return f.filePath; })));
        cmd.description = "generating dSYM for " + product.name;
        commands.push(cmd);
    }

    cmd = new Command(ModUtils.moduleProperty(product, "stripPath"),
                      ["-S", outputs.primary[0].filePath]);
    cmd.silent = true;
    commands.push(cmd);
    if (outputs.dynamiclibrary_symbols)
        Array.prototype.push.apply(commands, Gcc.createSymbolCheckingCommands(product, outputs));
    return commands;
}

