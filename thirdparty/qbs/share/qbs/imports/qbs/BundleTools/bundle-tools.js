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
var DarwinTools = require("qbs.DarwinTools");
var PropertyList = require("qbs.PropertyList");

function destinationDirectoryForResource(product, input) {
    var path = product.destinationDirectory;
    var inputFilePath = FileInfo.joinPaths(input.baseDir, input.fileName);
    var key = DarwinTools.localizationKey(inputFilePath);
    if (key) {
        path = FileInfo.joinPaths(path, localizedResourcesFolderPath(product, key));
        var subPath = DarwinTools.relativeResourcePath(inputFilePath);
        if (subPath && subPath !== '.')
            path = FileInfo.joinPaths(path, subPath);
    } else {
        path = FileInfo.joinPaths(path, product.moduleProperty("bundle", "unlocalizedResourcesFolderPath"));
    }
    return path;
}

function localizedResourcesFolderPath(product, key) {
    return FileInfo.joinPaths(product.moduleProperty("bundle", "unlocalizedResourcesFolderPath"), key + product.moduleProperty("bundle", "localizedResourcesFolderSuffix"));
}

function infoPlistContents(infoPlistFilePath) {
    if (infoPlistFilePath === undefined)
        return undefined;

    var plist = new PropertyList();
    try {
        plist.readFromFile(infoPlistFilePath);
        return plist.toObject();
    } finally {
        plist.clear();
    }
}

function infoPlistFormat(infoPlistFilePath) {
    if (infoPlistFilePath === undefined)
        return undefined;

    var plist = new PropertyList();
    try {
        plist.readFromFile(infoPlistFilePath);
        return plist.format();
    } finally {
        plist.clear();
    }
}
