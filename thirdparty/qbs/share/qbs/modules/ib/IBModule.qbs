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

import qbs.BundleTools
import qbs.DarwinTools
import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.Process
import 'ib.js' as Ib

Module {
    Depends { name: "xcode"; required: false }

    Probe {
        id: ibProbe
        property string toolPath: ibtoolPath // input
        property string toolVersion // output
        configure: {
            toolVersion = Ib.ibtoolVersion(toolPath);
            found = true;
        }
    }

    condition: qbs.hostOS.contains("darwin") && qbs.targetOS.contains("darwin")

    property bool warnings: true
    property bool errors: true
    property bool notices: true

    property stringList flags

    // tiffutil specific
    property string tiffutilName: "tiffutil"
    property string tiffutilPath: FileInfo.joinPaths("/usr/bin", tiffutilName)
    property bool combineHidpiImages: true

    // iconutil specific
    property string iconutilName: "iconutil"
    property string iconutilPath: FileInfo.joinPaths("/usr/bin", iconutilName)

    // XIB/NIB specific
    property string ibtoolName: "ibtool"
    property string ibtoolPath: FileInfo.joinPaths(xcode.developerPath, "/usr/bin", ibtoolName)
    property bool flatten: true
    property string module
    property bool autoActivateCustomFonts: true

    // Asset catalog specific
    property string actoolName: xcode.present ? "actool" : "ictool"
    property string actoolPath: FileInfo.joinPaths(xcode.developerPath, "/usr/bin", actoolName)
    property string appIconName
    property string launchImageName
    property bool compressPngs: true

    // private properties
    property string outputFormat: "human-readable-text"
    property string tiffSuffix: ".tiff"
    property string appleIconSuffix: ".icns"
    property string compiledAssetCatalogSuffix: ".car"
    property string compiledNibSuffix: ".nib"
    property string compiledStoryboardSuffix: ".storyboardc"

    version: ibtoolVersion
    property string ibtoolVersion: ibProbe.toolVersion
    property var ibtoolVersionParts: ibtoolVersion ? ibtoolVersion.split('.').map(function(item) { return parseInt(item, 10); }) : []
    property int ibtoolVersionMajor: ibtoolVersionParts[0]
    property int ibtoolVersionMinor: ibtoolVersionParts[1]
    property int ibtoolVersionPatch: ibtoolVersionParts[2]

    property stringList targetDevices: xcode.present
                                       ? xcode.targetDevices
                                       : DarwinTools.targetDevices(qbs.targetOS)

    validate: {
        var validator = new ModUtils.PropertyValidator("ib");
        validator.setRequiredProperty("ibtoolVersion", ibtoolVersion);
        validator.setRequiredProperty("ibtoolVersionMajor", ibtoolVersionMajor);
        validator.setRequiredProperty("ibtoolVersionMinor", ibtoolVersionMinor);
        validator.addVersionValidator("ibtoolVersion", ibtoolVersion, 2, 3);
        validator.addRangeValidator("ibtoolVersionMajor", ibtoolVersionMajor, 1);
        validator.addRangeValidator("ibtoolVersionMinor", ibtoolVersionMinor, 0);
        if (ibtoolVersionPatch !== undefined)
            validator.addRangeValidator("ibtoolVersionPatch", ibtoolVersionPatch, 0);
        validator.validate();
    }

    FileTagger {
        patterns: ["*.png"]
        fileTags: ["png"]
    }

    FileTagger {
        patterns: ["*.iconset"] // bundle
        fileTags: ["iconset"]
    }

    FileTagger {
        patterns: ["*.nib", "*.xib"]
        fileTags: ["nib"]
    }

    FileTagger {
        patterns: ["*.storyboard"]
        fileTags: ["storyboard"]
    }

    FileTagger {
        patterns: ["*.xcassets"] // bundle
        fileTags: ["assetcatalog"]
    }

    Rule {
        multiplex: true
        inputs: ["png"]

        outputFileTags: ["tiff"]
        outputArtifacts: Ib.tiffutilArtifacts(product, inputs)

        prepare: Ib.prepareTiffutil.apply(Ib, arguments)
    }

    Rule {
        inputs: ["iconset"]

        outputFileTags: ["icns", "bundle.input"]
        outputArtifacts: ([{
            filePath: FileInfo.joinPaths(product.destinationDirectory, input.completeBaseName +
                                         ModUtils.moduleProperty(product, "appleIconSuffix")),
            fileTags: ["icns", "bundle.input"],
            bundle: {
                _bundleFilePath: FileInfo.joinPaths(BundleTools.destinationDirectoryForResource(product, input),
                                                    input.completeBaseName +
                                                    ModUtils.moduleProperty(product, "appleIconSuffix"))
            }
        }])

        prepare: {
            var args = ["--convert", "icns", "--output", output.filePath, input.filePath];
            var cmd = new Command(ModUtils.moduleProperty(product, "iconutilPath"), args);
            cmd.description = "compiling " + input.fileName;
            return cmd;
        }
    }

    Rule {
        inputs: ["nib", "storyboard"]

        outputFileTags: {
            var tags = ["partial_infoplist"];
            for (var i = 0; i < inputs.length; ++i)
                tags = tags.uniqueConcat(ModUtils.allFileTags(Ib.ibtoolFileTaggers(inputs[i])));
            return tags;
        }

        outputArtifacts: Ib.ibtoolOutputArtifacts(product, inputs, input)

        prepare: {
            var cmd = new Command(ModUtils.moduleProperty(product, "ibtoolPath"),
                                  Ib.ibtooldArguments(product, inputs, input, outputs));
            cmd.description = "compiling " + input.fileName;

            // Also display the language name of the nib/storyboard being compiled if it has one
            var localizationKey = DarwinTools.localizationKey(input.filePath);
            if (localizationKey)
                cmd.description += ' (' + localizationKey + ')';

            cmd.highlight = 'compiler';

            // May not be strictly needed, but is set by some versions of Xcode
            if (input.fileTags.contains("storyboard"))
                cmd.environment.push("IBSC_MINIMUM_COMPATIBILITY_VERSION=" +
                                     (product.moduleProperty("cpp", "minimumDarwinVersion") || ""));

            cmd.stdoutFilterFunction = function(output) {
                return "";
            };

            return cmd;
        }
    }

    Rule {
        inputs: ["assetcatalog"]
        multiplex: true

        outputArtifacts: Ib.actoolOutputArtifacts(product, inputs)
        outputFileTags: ["bundle.input", "compiled_assetcatalog", "partial_infoplist"]

        prepare: {
            var mkdir = new JavaScriptCommand();
            mkdir.silent = true;
            mkdir.sourceCode = function () {
                File.makePath(FileInfo.joinPaths(product.buildDirectory, "actool.dir"));
            };

            var cmd = new Command(ModUtils.moduleProperty(product, "actoolPath"),
                                  Ib.ibtooldArguments(product, inputs, input, outputs));
            cmd.description = inputs["assetcatalog"].map(function (input) {
                return "compiling " + input.fileName;
            }).join('\n');
            cmd.highlight = "compiler";

            cmd.stdoutFilterFunction = function(output) {
                return "";
            };

            return [mkdir, cmd];
        }
    }
}
