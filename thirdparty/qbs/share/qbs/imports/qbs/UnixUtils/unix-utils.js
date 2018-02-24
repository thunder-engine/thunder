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

var FileInfo = require("qbs.FileInfo");

function soname(product, outputFileName) {
    var soVersion = product.moduleProperty("cpp", "soVersion");
    if (product.moduleProperty("qbs", "targetOS").contains("darwin")) {
        // If this is a bundle, ignore the parameter and use the relative path to the bundle binary
        // For example: qbs.framework/Versions/1/qbs
        if (product.moduleProperty("bundle", "isBundle"))
            outputFileName = product.moduleProperty("bundle", "executablePath");
    } else if (soVersion) {
        // For non-Darwin platforms, append the shared library major version number to the soname
        // For example: libqbscore.so.1
        var version = product.moduleProperty("cpp", "internalVersion");
        if (version) {
            outputFileName = outputFileName.substr(0, outputFileName.length - version.length)
                    + soVersion;
        } else {
            outputFileName += "." + soVersion;
        }
    }

    // Prepend the soname prefix
    // For example, @rpath/libqbscore.dylib or /usr/lib/libqbscore.so.1
    var prefix = product.moduleProperty("cpp", "sonamePrefix");
    if (prefix)
        outputFileName = FileInfo.joinPaths(prefix, outputFileName);

    return outputFileName;
}
