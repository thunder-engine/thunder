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

var FileInfo = require("qbs.FileInfo");
var Process = require("qbs.Process");

function scanQrc(qrcFilePath) {
    var absInputDir = FileInfo.path(qrcFilePath);
    var result = [];
    var process = new Process();
    try {
        var rcc = FileInfo.joinPaths(product.Qt.core.binPath, 'rcc' + product.cpp.executableSuffix);
        var exitCode = process.exec(rcc, ["--list", qrcFilePath], true);
        for (;;) {
            var line = process.readLine();
            if (!line)
                break;
            line = line.trim();
            line = FileInfo.relativePath(absInputDir, line);
            result.push(line);
        }
    } finally {
        process.close();
    }
    return result;
}

function qtQuickCompilerOutputName(filePath) {
    var str = filePath.replace(/\//g, '_');
    var i = str.lastIndexOf('.');
    if (i != -1)
        str = str.substr(0, i) + '_' + str.substr(i + 1);
    str += ".cpp";
    return str;
}

function qtQuickResourceFileOutputName(fileName) {
    return fileName.replace(/\.qrc$/, "_qtquickcompiler.qrc");
}

function contentFromQrc(qrcFilePath) {
    var filesInQrc = scanQrc(qrcFilePath);
    var qmlJsFiles = filesInQrc.filter(function (filePath) {
        return (/\.(js|qml)$/).test(filePath);
    } );
    var content = {};
    if (filesInQrc.length - qmlJsFiles.length > 0) {
        content.newQrcFileName = qtQuickResourceFileOutputName(input.fileName);
    }
    content.qmlJsFiles = qmlJsFiles.map(function (filePath) {
        return {
            input: filePath,
            output: qtQuickCompilerOutputName(filePath)
        };
    });
    return content;
}
