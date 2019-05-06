/****************************************************************************
**
** Copyright (C) 2018 Ivan Komissarov
** Contact: abbapoh@gmail.com
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

var checkPath = function(path) {
    return path && File.exists(path)
};

function toCamelCase(str){
    return str.split('_').map(function(word, index) {
        // If it is the first word make sure to lowercase all the chars.
        if (index === 0) {
            return word.toLowerCase();
        }
        // If it is not the first word only upper case the first char and lowercase the rest.
        return word.charAt(0).toUpperCase() + word.slice(1).toLowerCase();
    }).join('');
}

function getOutputDir(module, product, input)  {
    var outputDir = module.outputDir;
    var importPaths = module.importPaths;
    if (importPaths.length !== 0) {
        var canonicalInput = File.canonicalFilePath(FileInfo.path(input.filePath));
        for (var i = 0; i < importPaths.length; ++i) {
            path = File.canonicalFilePath(importPaths[i]);

            if (canonicalInput.startsWith(path)) {
                return outputDir + "/" + FileInfo.relativePath(path, canonicalInput);
            }
        }
    }
    return outputDir;
}

function cppArtifact(module, product, input, tag, suffix) {
    var outputDir = getOutputDir(module, product, input);
    return {
        fileTags: [tag],
        filePath: outputDir + "/" + FileInfo.baseName(input.fileName) + suffix,
        cpp: {
            includePaths: [].concat(input.cpp.includePaths, outputDir),
            warningLevel: "none",
        }
    };
}

function objcArtifact(module, product, input, tag, suffix) {
    var outputDir = getOutputDir(module, product, input);
    return {
        fileTags: [tag],
        filePath: outputDir + "/" + toCamelCase(FileInfo.baseName(input.fileName)) + suffix,
        cpp: {
            includePaths: [].concat(input.cpp.includePaths, outputDir),
            warningLevel: "none",
        }
    }
}

function doPrepare(module, product, input, outputs, lang)
{
    var outputDir = module.outputDir;
    var args = [];

    args.push("--" + lang + "_out", outputDir);

    var importPaths = module.importPaths;
    if (importPaths.length === 0)
        importPaths = [FileInfo.path(input.filePath)];
    importPaths.forEach(function(path) {
        if (!FileInfo.isAbsolutePath(path))
            path = FileInfo.joinPaths(product.sourceDirectory, path);
        args.push("--proto_path", path);
    });

    args.push(input.filePath);

    var cmd = new Command(module.protocBinary, args);
    cmd.highlight = "codegen";
    cmd.description = "generating " + lang + " files for " + input.fileName;
    return [cmd];
}
