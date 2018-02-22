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

var DarwinTools = require("qbs.DarwinTools");
var File = require("qbs.File");
var FileInfo = require("qbs.FileInfo");
var Process = require("qbs.Process");
var PropertyList = require("qbs.PropertyList");
var Utilities = require("qbs.Utilities");

var XcodeArchSpecsReader = (function () {
    function XcodeArchSpecsReader(specsPath) {
        var plist = new PropertyList();
        try {
            plist.readFromFile(specsPath);
            this.specsObject = plist.toObject();
        } finally {
            plist.clear();
        }
    }
    XcodeArchSpecsReader.prototype.getArchitectureSettings = function () {
        if (this.specsObject) {
            var names = [];
            for (var i = 0; i < this.specsObject.length; ++i) {
                var dict = this.specsObject[i];
                var name = dict["ArchitectureSetting"];
                if (name)
                    names.push(name);
            }
            return names;
        }
    };
    XcodeArchSpecsReader.prototype.getArchitectureSettingValue = function (settingName) {
        // settingName can be: ARCHS_STANDARD, ARCHS_STANDARD_32_BIT, ARCHS_STANDARD_64_BIT,
        // ARCHS_STANDARD_32_64_BIT, ARCHS_STANDARD_INCLUDING_64_BIT, or ARCHS_UNIVERSAL_IPHONE_OS.
        // NATIVE_ARCH_ACTUAL doesn't have a RealArchitectures property since it's determined by
        // Xcode programmatically.
        if (this.specsObject) {
            for (var i = 0; i < this.specsObject.length; ++i) {
                var dict = this.specsObject[i];
                if (dict["ArchitectureSetting"] === settingName) {
                    var realArchs = dict["RealArchitectures"];
                    if (realArchs) {
                        var effectiveRealArchs = [];
                        for (var j = 0; j < realArchs.length; ++j) {
                            var re = /^\$\(([A-Za-z0-9_]+)\)$/;
                            var match = realArchs[j].match(re);
                            if (match) {
                                var val = this.getArchitectureSettingValue(match[1]);
                                // Don't silently omit values if missing. For example, if
                                // ARCHS_FOO=[x86_64, $(ARCHS_BAR)], return 'undefined' instead of
                                // simply [x86_64]. Not known to have any real world occurrences.
                                if (!val)
                                    return undefined;
                                Array.prototype.push.apply(effectiveRealArchs, val);
                            } else {
                                effectiveRealArchs.push(realArchs[j]);
                            }
                        }
                        return effectiveRealArchs;
                    }
                }
            }
        }
    };
    return XcodeArchSpecsReader;
}());

function sdkInfoList(sdksPath) {
    var sdkInfo = [];
    var sdks = File.directoryEntries(sdksPath, File.Dirs | File.NoDotAndDotDot);
    for (var i in sdks) {
        // SDK directory name must contain a version number;
        // we don't want the versionless iPhoneOS.sdk directory for example
        if (!sdks[i].match(/[0-9]+/))
            continue;

        var settingsPlist = FileInfo.joinPaths(sdksPath, sdks[i], "SDKSettings.plist");
        var propertyList = new PropertyList();
        try {
            propertyList.readFromFile(settingsPlist);

            function checkPlist(plist) {
                if (!plist || !plist["CanonicalName"] || !plist["Version"])
                    return false;

                var re = /^([0-9]+)\.([0-9]+)$/;
                return plist["Version"].match(re);
            }

            var plist = propertyList.toObject();
            if (!checkPlist(plist)) {
                console.warn("Skipping corrupted SDK installation: "
                             + FileInfo.joinPaths(sdksPath, sdks[i]));
                continue;
            }

            sdkInfo.push(plist);
        } finally {
            propertyList.clear();
        }
    }

    // Sort by SDK version number
    sdkInfo.sort(function (a, b) {
        var re = /^([0-9]+)\.([0-9]+)$/;
        a = a["Version"].match(re);
        if (a)
            a = {major: a[1], minor: a[2]};
        b = b["Version"].match(re);
        if (b)
            b = {major: b[1], minor: b[2]};

        if (a.major === b.major)
            return a.minor - b.minor;
        return a.major - b.major;
    });

    return sdkInfo;
}

function findSigningIdentities(security, searchString) {
    var process;
    var identities;
    if (searchString) {
        try {
            process = new Process();
            if (process.exec(security, ["find-identity", "-p", "codesigning", "-v"], true) !== 0)
                console.error(process.readStdErr());

            var lines = process.readStdOut().split("\n");
            for (var i in lines) {
                // e.g. 1) AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA "Mac Developer: John Doe (XXXXXXXXXX) john.doe@example.org"
                var match = lines[i].match(/^\s*[0-9]+\)\s+([A-Fa-f0-9]{40})\s+"([^"]+)"$/);
                if (match !== null) {
                    var hexId = match[1];
                    var displayName = match[2];
                    if (hexId === searchString || displayName.startsWith(searchString)) {
                        if (!identities)
                            identities = [];
                        identities.push([hexId, displayName]);
                        break;
                    }
                }
            }
        } finally {
            process.close();
        }
    }
    return identities;
}

function provisioningProfilePlistContents(filePath) {
    if (filePath === undefined)
        return undefined;

    var plist = new PropertyList();
    try {
        plist.readFromFile(filePath);
        return plist.toObject();
    } finally {
        plist.clear();
    }
}

function archsSpecsPath(version, targetOS, platformType, platformPath, devicePlatformPath) {
    var _specsPluginBaseName;
    if (Utilities.versionCompare(version, "7") >= 0) {
        if (targetOS.contains("ios"))
            _specsPluginBaseName = "iOSPlatform";
        if (targetOS.contains("tvos"))
            _specsPluginBaseName = "AppleTV";
        if (targetOS.contains("watchos"))
            _specsPluginBaseName = "Watch";
    }

    var _archSpecsDir = _specsPluginBaseName
            ? FileInfo.joinPaths(devicePlatformPath, "Developer", "Library", "Xcode",
                                 "PrivatePlugIns",
                                 "IDE" + _specsPluginBaseName + "SupportCore.ideplugin", "Contents",
                                 "Resources")
            : FileInfo.joinPaths(platformPath, "Developer", "Library", "Xcode", "Specifications");

    var _archSpecsFileBaseName = targetOS.contains("ios")
            ? (targetOS.contains("ios-simulator") ? "iPhone Simulator " : "iPhoneOS")
            : DarwinTools.applePlatformDirectoryName(targetOS, platformType) + " ";

    if (_specsPluginBaseName) {
        switch (platformType) {
        case "device":
            return FileInfo.joinPaths(_archSpecsDir, "Device.xcspec");
        case "simulator":
            return FileInfo.joinPaths(_archSpecsDir, "Simulator.xcspec");
        }
    }

    return FileInfo.joinPaths(_archSpecsDir, _archSpecsFileBaseName + "Architectures.xcspec");
}
