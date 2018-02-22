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

var DarwinTools = require("qbs.DarwinTools");
var FileInfo = require("qbs.FileInfo");
var TextFile = require("qbs.TextFile");
var Utilities = require("qbs.Utilities");

function localizationFromArtifact(input) {
    var locale = input.dmg.licenseLocale || DarwinTools.localizationKey(input.filePath);
    if (!locale)
        throw("Could not determine localization for license file: " + input.filePath);
    return locale;
}

function dmgbuildSettings(product, inputs) {
    var backgroundImages = inputs["tiff"];
    var backgroundImage;
    if (backgroundImages) {
        if (backgroundImages.length > 1)
            throw new Error("only one background image may be specified");
        backgroundImage = backgroundImages[0].filePath;
    }

    var volumeIcons = inputs["icns"];
    var volumeIcon;
    if (volumeIcons) {
        if (volumeIcons.length > 1)
            throw new Error("only one volume icon may be specified");
        volumeIcon = volumeIcons[0].filePath;
    }

    var licenseFileObjects = Array.prototype.map.call(inputs["dmg.license"], function (a) {
        return {
            "dmg": {
                "licenseLocale": localizationFromArtifact(a),
                "licenseLanguageName": a.dmg.licenseLanguageName,
                "licenseAgreeButtonText": a.dmg.licenseAgreeButtonText,
                "licenseDisagreeButtonText": a.dmg.licenseDisagreeButtonText,
                "licensePrintButtonText": a.dmg.licensePrintButtonText,
                "licenseSaveButtonText": a.dmg.licenseSaveButtonText,
                "licenseInstructionText": a.dmg.licenseInstructionText,
            },
            "filePath": a.filePath
        };
    });

    function reduceLicensesForKey(licenseFileObjects, key) {
        return licenseFileObjects.reduce(function (accumulator, currentValue) {
            var locale = currentValue.dmg.licenseLocale;
            if (accumulator[locale])
                throw new Error("Multiple license files for localization '" + locale + "'");
            switch (key) {
            case "licenses":
                accumulator[locale] = currentValue.filePath;
                break;
            case "buttons":
                var texts = [
                    currentValue.dmg.licenseLanguageName,
                    currentValue.dmg.licenseAgreeButtonText,
                    currentValue.dmg.licenseDisagreeButtonText,
                    currentValue.dmg.licensePrintButtonText,
                    currentValue.dmg.licenseSaveButtonText,
                    currentValue.dmg.licenseInstructionText
                ];
                accumulator[locale] = texts.every(function (a) { return !!a; }) ? texts : undefined;
                break;
            }
            return accumulator;
        }, {});
    }

    var contentsArray = Array.prototype.map.call(inputs["dmg.input"], function (a) {
        if (a.dmg.sourceBase && !a.filePath.startsWith(a.dmg.sourceBase)) {
            throw new Error("Cannot install '" + a.filePath + "', " +
                            "because it doesn't start with the value of " +
                            "dmg.sourceBase '" + a.dmg.sourceBase + "'.");
        }

        var isSymlink = a.fileTags.contains("dmg.input.symlink");
        return {
            "x": a.dmg.iconX,
            "y": a.dmg.iconY,
            "type": isSymlink ? "link" : "file",
            "path": isSymlink ? a.dmg.symlinkTarget : a.filePath,
            "name": FileInfo.relativePath(a.dmg.sourceBase || FileInfo.path(a.filePath), a.filePath)
        };
    });

    Array.prototype.forEach.call(product.dmg.iconPositions, function (obj) {
        var existingIndex = -1;
        Array.prototype.forEach.call(contentsArray, function (contentsItem, i) {
            if (contentsItem["name"] === obj["path"])
                existingIndex = i;
        });

        if (existingIndex >= 0) {
            contentsArray[existingIndex]["x"] = obj["x"];
            contentsArray[existingIndex]["y"] = obj["y"];
        } else {
            contentsArray.push({
                "type": "position",
                "name": obj["path"], // name => path is not a typo
                "path": obj["path"],
                "x": obj["x"],
                "y": obj["y"]
            });
        }
    });

    return {
        "title": product.dmg.volumeName,
        "icon": !product.dmg.badgeVolumeIcon ? volumeIcon : undefined,
        "badge-icon": product.dmg.badgeVolumeIcon ? volumeIcon : undefined,
        "background": backgroundImage,
        "background-color": product.dmg.backgroundColor,
        "icon-size": product.dmg.iconSize,
        "window": {
            "position": {
                "x": product.dmg.windowX,
                "y": product.dmg.windowY
            },
            "size": {
                "width": product.dmg.windowWidth,
                "height": product.dmg.windowHeight
            }
        },
        "format": product.dmg.format,
        "compression-level": product.dmg.compressionLevel,
        "license": {
            "default-language": product.dmg.defaultLicenseLocale,
            "licenses": reduceLicensesForKey(licenseFileObjects, "licenses"),
            "buttons": reduceLicensesForKey(licenseFileObjects, "buttons")
        },
        "contents": contentsArray
    };
}

function prepareLicense(project, product, inputs, outputs, input, output) {
    var cmd = new Command(product.dmg.textutilPath, [
        "-convert", "rtf",
        "-strip",
        "-font", "Arial",
        "-output", output.filePath,
        "--", input.filePath
    ]);
    cmd.description = "converting " + input.fileName;
    return [cmd];
}

function prepareDmg(project, product, inputs, outputs, input, output) {
    var i;
    var cmd;
    var cmds = [];

    var settingsJsonFilePath = FileInfo.joinPaths(product.destinationDirectory,
                                                  "settings.json");
    cmd = new JavaScriptCommand();
    cmd.silent = true;
    cmd.settingsJSON = dmgbuildSettings(product, inputs);
    cmd.settingsJsonFilePath = settingsJsonFilePath;
    cmd.sourceCode = function () {
        var tf;
        try {
            tf = new TextFile(settingsJsonFilePath, TextFile.WriteOnly);
            tf.writeLine(JSON.stringify(settingsJSON, undefined, 4));
        } finally {
            if (tf)
                tf.close();
        }
    }
    cmds.push(cmd);

    // Create the actual DMG via dmgbuild
    cmd = new Command(FileInfo.joinPaths(product.qbs.libexecPath, "dmgbuild"),
                      [product.dmg.volumeName,
                       output.filePath,
                       "--no-hidpi", // qbs handles this by itself
                       "--settings", settingsJsonFilePath]);
    cmd.environment = ["PYTHONPATH=" + product.dmg.pythonPath];
    cmd.description = "creating " + output.fileName;
    cmds.push(cmd);

    return cmds;
}
