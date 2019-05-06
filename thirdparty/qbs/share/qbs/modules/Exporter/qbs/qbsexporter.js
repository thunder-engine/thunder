/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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
var ModUtils = require("qbs.ModUtils");

function tagListToString(tagList)
{
    return JSON.stringify(tagList);
}

function stringToTagList(tagListString)
{
    return JSON.parse(tagListString);
}

function writeTargetArtifactGroup(output, tagList, artifactList, moduleInstallDir, moduleFile)
{
    // Do not add our qbs module file itself.
    if (tagListToString(tagList) === tagListToString(output.fileTags))
        return;

    moduleFile.writeLine("    Group {");
    moduleFile.writeLine("        filesAreTargets: true");
    var filteredTagList = tagList.filter(function(t) { return t !== "installable"; });
    moduleFile.writeLine("        fileTags: " + JSON.stringify(filteredTagList));
    moduleFile.writeLine("        files: [");
    for (i = 0; i < artifactList.length; ++i) {
        var artifact = artifactList[i];
        var installedArtifactFilePath = ModUtils.artifactInstalledFilePath(artifact);

        // Use relative file paths for relocatability.
        var relativeInstalledArtifactFilePath = FileInfo.relativePath(moduleInstallDir,
                                                                      installedArtifactFilePath);
        moduleFile.writeLine("            " + JSON.stringify(relativeInstalledArtifactFilePath)
                             + ",");
    }
    moduleFile.writeLine("        ]");
    moduleFile.writeLine("    }");

}

function writeTargetArtifactGroups(product, output, moduleFile)
{
    var relevantArtifacts = [];
    for (var i = 0; i < (product.Exporter.qbs._artifactTypes || []).length; ++i) {
        var tag = product.Exporter.qbs._artifactTypes[i];
        var artifactsForTag = product.artifacts[tag] || [];
        for (var j = 0; j < artifactsForTag.length; ++j) {
            if (!relevantArtifacts.contains(artifactsForTag[j]))
                relevantArtifacts.push(artifactsForTag[j]);
        }
    }
    var artifactsByTags = {};
    var artifactCount = relevantArtifacts ? relevantArtifacts.length : 0;
    for (i = 0; i < artifactCount; ++i) {
        var artifact = relevantArtifacts[i];
        if (!artifact.fileTags.contains("installable"))
            continue;

        // Put all artifacts with the same set of file tags into the same group, so we don't
        // create more groups than necessary.
        var key = tagListToString(artifact.fileTags);
        var currentList = artifactsByTags[key];
        if (currentList)
            currentList.push(artifact);
        else
            currentList = [artifact];
        artifactsByTags[key] = currentList;
    }
    var moduleInstallDir = FileInfo.path(ModUtils.artifactInstalledFilePath(output));
    for (var tagListKey in artifactsByTags) {
        writeTargetArtifactGroup(output, stringToTagList(tagListKey), artifactsByTags[tagListKey],
                   moduleInstallDir, moduleFile);
    }
}

function checkValuePrefix(name, value, forbiddenPrefix, prefixDescription)
{
    if (value.startsWith(forbiddenPrefix)) {
        throw "Value '" + value + "' for exported property '" + name + "' in product '"
                + product.name + "' points into " + prefixDescription + ".\n"
                + "Did you forget to set the prefixMapping property in an Export item?";
    }
}

function stringifyValue(project, product, moduleInstallDir, prop, value)
{
    if (Array.isArray(value)) {
        var repr = "[";
        for (var i = 0; i < value.length; ++i) {
            repr += stringifyValue(project, product, moduleInstallDir, prop, value[i]) + ", ";
        }
        repr += "]";
        return repr;
    }
    if (typeof(value) !== "string") {
        var value = JSON.stringify(value);
        if (prop.type === "variant")
            value = '(' + value + ')';
        return value;
    }

    // Catch user oversights: Paths that point into the project source or build directories
    // make no sense in the module.
    if (!value.startsWith(product.qbs.installRoot)) {
        checkValuePrefix(prop.name, value, project.buildDirectory, "project build directory");
        checkValuePrefix(prop.name, value, project.sourceDirectory, "project source directory");
    }

    // Adapt file paths pointing into the install dir, that is, make them relative to the
    // module file for relocatability. We accept them with or without the install root.
    // The latter form will typically be a result of applying the prefixMapping property,
    // while the first one could be an untransformed path, for instance if the project
    // file is written in such a way that include paths are picked up from the installed
    // location rather than the source directory.
    var valuePrefixToStrip;
    var fullInstallPrefix = FileInfo.joinPaths(product.qbs.installRoot, product.qbs.installPrefix);
    if (fullInstallPrefix.length > 1 && value.startsWith(fullInstallPrefix)) {
        valuePrefixToStrip = fullInstallPrefix;
    } else {
        var installPrefix = FileInfo.joinPaths("/", product.qbs.installPrefix);
        if (installPrefix.length > 1 && value.startsWith(installPrefix))
            valuePrefixToStrip = installPrefix;
    }
    if (valuePrefixToStrip) {
        var deployedModuleInstallDir = moduleInstallDir.slice(fullInstallPrefix.length);
        return "FileInfo.cleanPath(FileInfo.joinPaths(path, FileInfo.relativePath("
                + JSON.stringify(deployedModuleInstallDir) + ", "
                + JSON.stringify(value.slice(valuePrefixToStrip.length) || "/") + ")))";
    }

    return JSON.stringify(value);
}

function writeProperty(project, product, moduleInstallDir, prop, indentation, considerValue,
                       moduleFile)
{
    var line = indentation;
    var separatorIndex = prop.name.lastIndexOf(".");
    var isModuleProperty = separatorIndex !== -1;
    var needsDeclaration = !prop.isBuiltin && !isModuleProperty;
    if (needsDeclaration)
        line += "property " + prop.type +  " ";
    var moduleName;
    if (isModuleProperty) {
        moduleName = prop.name.slice(0, separatorIndex);
        if ((product.Exporter.qbs.excludedDependencies || []).contains(moduleName))
            return;
    }
    line += prop.name + ": ";

    // We emit the literal value, unless the source code clearly refers to values from inside the
    // original project, in which case the evaluated value is used.
    if (considerValue && /(project|product)\./.test(prop.sourceCode)) {
        var value;
        if (isModuleProperty) {
            var propertyName = prop.name.slice(separatorIndex + 1);
            value = product.exports[moduleName][propertyName];
        } else {
            value = product.exports[prop.name];
        }
        line += stringifyValue(project, product, moduleInstallDir, prop, value);
    } else {
        line += prop.sourceCode.replace(/importingProduct\./g, "product.");
    }
    moduleFile.writeLine(line);
}

function writeProperties(project, product, moduleInstallDir, list, indentation, considerValue,
                         moduleFile)
{
    for (var i = 0; i < list.length; ++i) {
        writeProperty(project, product, moduleInstallDir, list[i], indentation, considerValue,
                      moduleFile);
    }
}

// This writes properties set on other modules in the Export item, i.e. property assignments
// like "cpp.includePaths: '...'".
function writeModuleProperties(project, product, output, moduleFile)
{
    var moduleInstallDir = FileInfo.path(ModUtils.artifactInstalledFilePath(output));
    var filteredProps = product.exports.properties.filter(function(p) {
        return p.name !== "name";
    });

    // The right-hand side can refer to values from the exporting product, in which case
    // the evaluated value, rather than the source code, needs to go into the module file.
    var considerValues = true;
    writeProperties(project, product, moduleInstallDir, filteredProps, "    ", considerValues,
                    moduleFile);
}

function writeItem(product, item, indentation, moduleFile)
{
    moduleFile.writeLine(indentation + item.name + " {");
    var newIndentation = indentation + "    ";

    // These are sub-items of the Export item, whose properties entirely live in the context
    // of the importing product. Therefore, they must never use pre-evaluated values.
    var considerValues = false;
    writeProperties(undefined, product, undefined, item.properties, newIndentation, considerValues,
                    moduleFile)

    for (var i = 0; i < item.childItems.length; ++i)
        writeItem(product, item.childItems[i], newIndentation, moduleFile);
    moduleFile.writeLine(indentation + "}");
}

function isExcludedDependency(product, childItem)
{
    if ((product.Exporter.qbs.excludedDependencies || []).length === 0)
        return false;
    if (childItem.name !== "Depends")
        return false;
    for (var i = 0; i < childItem.properties.length; ++i) {
        var prop = childItem.properties[i];
        var unquotedRhs = prop.sourceCode.slice(1, -1);
        if (prop.name === "name" && product.Exporter.qbs.excludedDependencies.contains(unquotedRhs))
            return true;
    }
    return false;
}

function writeChildItems(product, moduleFile)
{
    for (var i = 0; i < product.exports.childItems.length; ++i) {
        var item = product.exports.childItems[i];
        if (!isExcludedDependency(product, item))
            writeItem(product, item, "    ", moduleFile);
    }
}

function writeImportStatements(product, moduleFile)
{
    var imports = product.exports.imports;

    // We potentially use FileInfo ourselves when transforming paths in stringifyValue().
    if (!imports.contains("import qbs.FileInfo"))
        imports.push("import qbs.FileInfo");

    for (var i = 0; i < product.exports.imports.length; ++i)
        moduleFile.writeLine(product.exports.imports[i]);
}
