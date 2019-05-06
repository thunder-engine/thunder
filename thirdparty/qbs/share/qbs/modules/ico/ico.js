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
var Process = require("qbs.Process");

function prepareIconset(project, product, inputs, outputs, input, output) {
    inputs = File.directoryEntries(input.filePath, File.Files).map(function (p) {
        return {
            filePath: FileInfo.joinPaths(input.filePath, p),
            fileTags: p.endsWith(".png") ? ["png"] : [],
            ico: {}
        };
    });
    inputs = {"png": inputs.filter(function (a) { return a.fileTags.contains("png"); })};
    input = undefined;
    return prepare(project, product, inputs, outputs, input, output);
}

function prepare(project, product, inputs, outputs, input, output) {
    var args = ["--create", "--output=" + output.filePath];
    if (output.fileTags.contains("ico")) {
        args.push("--icon");
        if (product.ico.alphaThreshold !== undefined)
            args.push("--alpha-threshold=" + product.ico.alphaThreshold);
    }

    var isCursor = output.fileTags.contains("cur");
    if (isCursor)
        args.push("--cursor");

    var hasMultipleImages = inputs["png"].length > 1;
    inputs["png"].map(function(inp) {
        if (isCursor) {
            var hasX = inp.ico.cursorHotspotX !== undefined;
            var hasY = inp.ico.cursorHotspotY !== undefined;
            if (hasX || hasY) {
                if (hasMultipleImages && product.ico.hasCursorHotspotBug) {
                    console.warn("icotool " + product.ico.version + " does not support setting " +
                                 "the hotspot for cursor files with multiple images. Install " +
                                 "icoutils 0.32.0 or newer to use this feature.");
                } else {
                    if (hasX)
                        args.push("--hotspot-x=" + inp.ico.cursorHotspotX);
                    if (hasY)
                        args.push("--hotspot-y=" + inp.ico.cursorHotspotY);
                }
            }
        }
        if (inp.ico.raw)
            args.push("-r");
        args.push(inp.filePath);
    });

    var cmd = new Command(product.ico.icotoolFilePath, args);
    cmd.description = "creating " + output.fileName;
    return [cmd];
}

function findIcoUtilsVersion(toolFilePath) {
    var p = new Process();
    try {
        p.exec(toolFilePath, ["--version"]);
        var re = /^[a-z]+ \(icoutils\) ([0-9]+(?:\.[0-9]+){1,2})$/m;
        var match = p.readStdOut().trim().match(re);
        if (match !== null)
            return match[1];
    } finally {
        p.close();
    }
}
