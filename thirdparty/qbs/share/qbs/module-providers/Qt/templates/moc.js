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

var ModUtils = require("qbs.ModUtils");

function args(product, input, outputFileName)
{
    var defines = product.cpp.compilerDefinesByLanguage;
    if (input.fileTags.contains("objcpp"))
        defines = ModUtils.flattenDictionary(defines["objcpp"]) || [];
    else if (input.fileTags.contains("cpp"))
        defines = ModUtils.flattenDictionary(defines["cpp"]) || [];
    else
        defines = [];
    defines = defines.uniqueConcat(product.cpp.platformDefines);
    defines = defines.uniqueConcat(input.cpp.defines);
    var includePaths = input.cpp.includePaths;
    includePaths = includePaths.uniqueConcat(input.cpp.systemIncludePaths);
    var useCompilerPaths = product.Qt.core.versionMajor >= 5;
    if (useCompilerPaths) {
        includePaths = includePaths.uniqueConcat(input.cpp.compilerIncludePaths);
    }
    var frameworkPaths = product.cpp.frameworkPaths;
    frameworkPaths = frameworkPaths.uniqueConcat(
                product.cpp.systemFrameworkPaths);
    if (useCompilerPaths) {
        frameworkPaths = frameworkPaths.uniqueConcat(
                    product.cpp.compilerFrameworkPaths);
    }
    var pluginMetaData = product.Qt.core.pluginMetaData;
    var args = [];
    args = args.concat(
                defines.map(function(item) { return '-D' + item; }),
                includePaths.map(function(item) { return '-I' + item; }),
                frameworkPaths.map(function(item) { return '-F' + item; }),
                pluginMetaData.map(function(item) { return '-M' + item; }),
                product.Qt.core.mocFlags,
                '-o', outputFileName,
                input.filePath);
    return args;
}

function fullPath(product)
{
    return product.Qt.core.binPath + '/' + product.Qt.core.mocName;
}

function outputArtifacts(project, product, inputs, input)
{
    var mocInfo = QtMocScanner.apply(input);
    if (!mocInfo.hasQObjectMacro)
        return [];
    var artifact = { fileTags: ["unmocable"] };
    if (mocInfo.hasPluginMetaDataMacro)
        artifact.explicitlyDependsOn = ["qt_plugin_metadata"];
    if (input.fileTags.contains("hpp")) {
        artifact.filePath = input.Qt.core.generatedHeadersDir
                + "/moc_" + input.completeBaseName + ".cpp";
        var amalgamate = input.Qt.core.combineMocOutput;
        artifact.fileTags.push(mocInfo.mustCompile ? (amalgamate ? "moc_cpp" : "cpp") : "hpp");
    } else {
        artifact.filePath = input.Qt.core.generatedHeadersDir + '/'
                + input.completeBaseName + ".moc";
        artifact.fileTags.push("hpp");
    }
    return [artifact];
}

function commands(project, product, inputs, outputs, input, output)
{
    var cmd = new Command(fullPath(product), args(product, input, output.filePath));
    cmd.description = 'moc ' + input.fileName;
    cmd.highlight = 'codegen';
    return cmd;
}
