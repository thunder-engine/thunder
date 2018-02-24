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

function winapiFamilyDefine(name) {
    return {
        "desktop": "DESKTOP_APP",
        "phone": "PHONE_APP",
        "pc": "PC_APP",
        "server": "SERVER",
        "system": "SYSTEM"
    }[name];
}

function winapiPartitionDefine(name) {
    return {
        "app": "APP",
        "desktop": "DESKTOP",
        "phone": "PHONE_APP",
        "pc": "PC_APP",
        "server": "SERVER",
        "system": "SYSTEM"
    }[name];
}

function characterSetDefines(charset) {
    var defines = [];
    if (charset === "unicode")
        defines.push("UNICODE", "_UNICODE");
    else if (charset === "mbcs")
        defines.push("_MBCS");
    return defines;
}

function canonicalizeVersion(version) {
    switch (version) {
    case "7":
        return "6.1";
    case "8":
        return "6.2";
    case "8.1":
        return "6.3";
    default:
        return version;
    }
}

function knownWindowsVersions() {
    // Add new Windows versions to this list when they are released
    return ['10.0', '6.3', '6.2', '6.1', '6.0', '5.2', '5.1', '5.0', '4.0'];
}

function isValidWindowsVersion(systemVersion) {
    var realVersions = knownWindowsVersions();
    for (i in realVersions)
        if (systemVersion === realVersions[i])
            return true;

    return false;
}

function getWindowsVersionInFormat(systemVersion, format) {
    if (!systemVersion)
        return undefined;

    var major = parseInt(systemVersion.split('.')[0], 10);
    var minor = parseInt(systemVersion.split('.')[1], 10);

    switch (format) {
    case "hex":
        // https://msdn.microsoft.com/en-us/library/6sehtctf.aspx
        return "0x" + ("0000" + ((major << 8) | minor).toString(16)).slice(-4);
    case "subsystem":
        // https://msdn.microsoft.com/en-us/library/fcc1zstk.aspx
        return major + '.' + (minor < 10 ? '0' : '') + minor;
    default:
        throw ("Unrecognized Windows version format " + format + ". Must be in {hex, subsystem}.");
    }
}
