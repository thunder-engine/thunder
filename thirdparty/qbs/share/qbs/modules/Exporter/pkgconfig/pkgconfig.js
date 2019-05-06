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

function quote(value)
{
    if (value.contains(" ") || value.contains("'") || value.contains('"')) {
        return '"' + value.replace(/(["'\\])/g, "\\$1") + '"';
    }
    return value;
}

function writeEntry(product, file, key, propertyName, required, additionalValues)
{
    var value = product.Exporter.pkgconfig[propertyName];
    if (additionalValues && additionalValues.length > 0)
        value = (value || []).concat(additionalValues);
    var valueIsNotEmpty = value && (!Array.isArray(value) || value.length > 0);
    if (valueIsNotEmpty) {
        if (Array.isArray(value))
            value = value.join(' ');
        file.writeLine(key + ": " + value);
    } else if (required) {
        throw "Failure creating " + FileInfo.fileName(file.filePath()) + ": The entry '" + key
                + "' is required, but property Exporter.pkgconfig."
                + propertyName + " is not set.";
    }
}

function collectAutodetectedData(topLevelProduct)
{
    var data = {
        libs: [],
        cflags: [],
        requires: [],
        requiresPrivate: []
    };
    if (!topLevelProduct.Exporter.pkgconfig.autoDetect)
        return data;

    var excludedDeps = topLevelProduct.Exporter.pkgconfig.excludedDependencies || [];
    var explicitRequires = topLevelProduct.Exporter.pkgconfig.requiresEntry || [];
    var explicitRequiresPrivate = topLevelProduct.Exporter.pkgconfig.requiresPrivateEntry || [];

    var transformFunc = topLevelProduct.Exporter.pkgconfig.transformFunction;

    // Make use of the "prefix" convenience variable if applicable.
    function quoteAndPrefixify(value)
    {
        var quotedValue = quote(value);
        var installPrefix = topLevelProduct.qbs.installPrefix || "";
        if (!topLevelProduct.Exporter.pkgconfig._usePrefix || typeof value !== "string"
                || !value.startsWith(installPrefix)
                || (value.length > installPrefix.length && value[installPrefix.length] !== '/')) {
            return quotedValue;
        }
        return quotedValue.replace(product.qbs.installPrefix, "${prefix}");
    }

    function transformedValue(product, moduleName, propertyName)
    {
        var originalValue = product.exports[moduleName][propertyName];
        var value = transformFunc
                ? eval("(" + transformFunc + ")(product, moduleName, propertyName, originalValue)")
                : originalValue;
        if (Array.isArray(value))
            value.forEach(function(v, i, a) { a[i] = quoteAndPrefixify(v); });
        else if (value)
            value = quoteAndPrefixify(value);
        return value;
    }

    function collectLibs(productOrModule)
    {
        var libs = [];
        var libArtifacts;
        var isProduct = !productOrModule.present;
        var considerDynamicLibs = !isProduct || (productOrModule.type
                && productOrModule.type.contains("dynamiclibrary"));
        if (considerDynamicLibs) {
            libArtifacts = productOrModule.artifacts.dynamiclibrary;
        } else {
            var considerStaticLibs = !isProduct || (productOrModule.type
                    && productOrModule.type.contains("staticlibrary"));
            if (considerStaticLibs)
                libArtifacts = productOrModule.artifacts.staticlibrary;
        }
        for (var i = 0; i < (libArtifacts || []).length; ++i) {
            var libArtifact = libArtifacts[i];
            if (libArtifact.qbs.install) {
                var installDir = FileInfo.path(ModUtils.artifactInstalledFilePath(libArtifact));
                installDir = installDir.slice(libArtifact.qbs.installRoot.length);
                libs.push("-L" + quoteAndPrefixify(FileInfo.cleanPath(installDir)),
                          "-l" + quote(productOrModule.targetName));
            }
        }
        if (!productOrModule.exports.cpp)
            return libs;
        var libPaths = transformedValue(productOrModule, "cpp", "libraryPaths");
        if (libPaths)
            libs.push.apply(libs, libPaths.map(function(p) { return "-L" + p; }));
        function libNamesToLibEntries(libNames) {
            return libNames.map(function(libName) { return "-l" + libName; });
        };
        var dlls = transformedValue(productOrModule, "cpp", "dynamicLibraries");
        if (dlls)
            libs.push.apply(libs, libNamesToLibEntries(dlls));
        var staticLibs = transformedValue(productOrModule, "cpp", "staticLibraries");
        if (staticLibs)
            libs.push.apply(libs, libNamesToLibEntries(staticLibs));
        var lFlags = transformedValue(productOrModule, "cpp", "linkerFlags");
        if (lFlags)
            libs.push.apply(libs, lFlags);
        lFlags = transformedValue(productOrModule, "cpp", "driverFlags");
        if (lFlags)
            libs.push.apply(libs, lFlags);
        lFlags = transformedValue(productOrModule, "cpp", "driverLinkerFlags");
        if (lFlags)
            libs.push.apply(libs, lFlags);
        return libs;
    }

    function collectCFlags(productOrModule)
    {
        if (!productOrModule.exports.cpp)
            return [];
        var flags = [];
        var defs = transformedValue(productOrModule, "cpp", "defines");
        if (defs)
            flags.push.apply(flags, defs.map(function(d) { return "-D" + d; }));
        var incPaths = transformedValue(productOrModule, "cpp", "includePaths");
        if (incPaths)
            flags.push.apply(flags, incPaths.map(function(p) { return "-I" + p; }));
        var cflags = transformedValue(productOrModule, "cpp", "commonCompilerFlags");
        if (cflags)
            flags.push.apply(flags, cflags);
        cflags = transformedValue(productOrModule, "cpp", "driverFlags");
        if (cflags)
            flags.push.apply(flags, cflags);
        cflags = transformedValue(productOrModule, "cpp", "cxxFlags")
                || transformedValue(productOrModule, "cpp", "cFlags");
        if (cflags)
            flags.push.apply(flags, cflags);
        return flags;
    }

    function collectAutodetectedDataRecursive(productOrModule, privateContext)
    {
        if (!privateContext) {
            data.libs.push.apply(data.libs, collectLibs(productOrModule));
            data.cflags.push.apply(data.cflags, collectCFlags(productOrModule));
        }
        var exportedDeps = productOrModule.exports ? productOrModule.exports.dependencies : [];
        var exportedDepNames = [];
        var privateDeps = [];
        for (var i = 0; i < exportedDeps.length; ++i)
            exportedDepNames.push(exportedDeps[i].name);
        for (i = 0; i < (productOrModule.dependencies || []).length; ++i) {
            var dep = productOrModule.dependencies[i];
            if (exportedDepNames.contains(dep.name))
                continue;
            privateDeps.push(dep);
        }

        function gatherData(dep) {
            if (dep.name === "Exporter.pkgconfig")
                return;
            var depHasPkgConfig = dep.Exporter && dep.Exporter.pkgconfig;
            if (depHasPkgConfig) {
                var entry = FileInfo.completeBaseName(dep.Exporter.pkgconfig.fileName);
                if (excludedDeps.contains(entry))
                    return;
                if (isPrivateDep && !data.requiresPrivate.contains(entry)
                        && !explicitRequiresPrivate.contains(entry)) {
                    data.requiresPrivate.push(entry);
                }
                if (!isPrivateDep && !data.requires.contains(entry)
                        && !explicitRequires.contains(entry)) {
                    data.requires.push(entry);
                }
            } else {
                if (excludedDeps.contains(dep.name))
                    return;
                if (isPrivateDep && explicitRequiresPrivate.contains(dep.name))
                    return;
                if (!isPrivateDep && explicitRequires.contains(dep.name))
                    return;
                collectAutodetectedDataRecursive(dep, isPrivateDep);
            }
        }
        var isPrivateDep = privateContext;
        exportedDeps.forEach(gatherData);
        isPrivateDep = true;
        privateDeps.forEach(gatherData);
    }

    collectAutodetectedDataRecursive(topLevelProduct, false);
    return data;
}
