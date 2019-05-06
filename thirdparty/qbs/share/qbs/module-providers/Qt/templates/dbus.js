/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

function outputFileName(input, suffix)
{
    var parts = input.fileName.split('.').filter(function(s) { return s.length > 0; });
    if (parts.length === 0)
        throw "Cannot run qdbusxml2cpp on '" + input.filePath + "': Unsuitable file name.";
    var outputBaseName = parts.length === 1 ? parts[0] : parts[parts.length - 2];
    return outputBaseName.toLowerCase() + suffix;
}

function createCommands(product, input, outputs, option)
{
    var exe = ModUtils.moduleProperty(product, "binPath") + '/'
            + ModUtils.moduleProperty(product, "xml2cppName");
    var hppOutput = outputs["hpp"][0];
    var hppArgs = ModUtils.moduleProperty(product, "xml2CppHeaderFlags");
    hppArgs.push(option, hppOutput.fileName + ':', input.filePath); // Can't use filePath on Windows
    var hppCmd = new Command(exe, hppArgs)
    hppCmd.description = "qdbusxml2cpp " + input.fileName + " -> " + hppOutput.fileName;
    hppCmd.highlight = "codegen";
    hppCmd.workingDirectory = FileInfo.path(hppOutput.filePath);
    var cppOutput = outputs["cpp"][0];
    var cppArgs = ModUtils.moduleProperty(product, "xml2CppSourceFlags");
    cppArgs.push("-i", hppOutput.filePath, option, ':' + cppOutput.fileName, input.filePath);
    var cppCmd = new Command(exe, cppArgs)
    cppCmd.description = "qdbusxml2cpp " + input.fileName + " -> " + cppOutput.fileName;
    cppCmd.highlight = "codegen";
    cppCmd.workingDirectory = FileInfo.path(cppOutput.filePath);
    return [hppCmd, cppCmd];
}
