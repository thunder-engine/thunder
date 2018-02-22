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

function _bundleExecutableTemporaryFilePath(product, variantSuffix) {
    if (variantSuffix === undefined)
        variantSuffix = product.cpp.variantSuffix;
    return ".tmp/" + FileInfo.fileName(bundleExecutableFilePath(product, variantSuffix));
}

function bundleExecutableFilePath(product, variantSuffix) {
    if (variantSuffix === undefined)
        variantSuffix = product.cpp.variantSuffix;
    return product.moduleProperty("bundle", "executablePath") + (variantSuffix || "");
}

function applicationFilePath(product, variantSuffix) {
    if (variantSuffix === undefined)
        variantSuffix = product.cpp.variantSuffix;
    if (product.moduleProperty("bundle", "isBundle"))
        return _bundleExecutableTemporaryFilePath(product, variantSuffix);

    return product.moduleProperty("cpp", "executablePrefix")
         + product.targetName + (variantSuffix || "")
         + product.moduleProperty("cpp", "executableSuffix");
}

function loadableModuleFilePath(product, variantSuffix) {
    if (variantSuffix === undefined)
        variantSuffix = product.cpp.variantSuffix;
    if (product.moduleProperty("bundle", "isBundle"))
        return _bundleExecutableTemporaryFilePath(product, variantSuffix);

    return product.moduleProperty("cpp", "loadableModulePrefix")
         + product.targetName + (variantSuffix || "")
         + product.moduleProperty("cpp", "loadableModuleSuffix");
}

function staticLibraryFilePath(product, variantSuffix) {
    if (variantSuffix === undefined)
        variantSuffix = product.cpp.variantSuffix;
    if (product.moduleProperty("bundle", "isBundle"))
        return _bundleExecutableTemporaryFilePath(product, variantSuffix);

    return product.moduleProperty("cpp", "staticLibraryPrefix")
         + product.targetName + (variantSuffix || "")
         + product.moduleProperty("cpp", "staticLibrarySuffix");
}

function dynamicLibraryFilePath(product, variantSuffix, version, maxParts) {
    if (variantSuffix === undefined)
        variantSuffix = product.cpp.variantSuffix;
    if (product.moduleProperty("bundle", "isBundle"))
        return _bundleExecutableTemporaryFilePath(product, variantSuffix);

    // If no override version was given, use the product's version
    // We specifically want to differentiate between undefined and i.e.
    // empty string as empty string should be taken to mean "no version"
    // and undefined should be taken to mean "use the product's version"
    // (which could still end up being "no version")
    if (version === undefined)
        version = product.moduleProperty("cpp", "internalVersion");

    // If we have a version number, potentially strip off some components
    if (maxParts === 0)
        version = undefined;
    else if (maxParts && version)
        version = version.split('.').slice(0, maxParts).join('.');

    // Start with prefix + name (i.e. libqbs, qbs)
    var fileName = product.moduleProperty("cpp", "dynamicLibraryPrefix")
            + product.targetName
            + (variantSuffix || "");

    // For Mach-O images, append the version number if there is one (i.e. libqbs.1.0.0)
    var imageFormat = product.moduleProperty("cpp", "imageFormat");
    if (version && imageFormat === "macho") {
        fileName += "." + version;
        version = undefined;
    }

    // Append the suffix (i.e. libqbs.1.0.0.dylib, libqbs.so, qbs.dll)
    fileName += product.moduleProperty("cpp", "dynamicLibrarySuffix");

    // For ELF images, append the version number if there is one (i.e. libqbs.so.1.0.0)
    if (version && imageFormat === "elf")
        fileName += "." + version;

    return fileName;
}

function linkerOutputFilePath(fileTag, product, variantSuffix, version, maxParts) {
    switch (fileTag) {
    case "application":
        return applicationFilePath(product, variantSuffix);
    case "loadablemodule":
        return loadableModuleFilePath(product, variantSuffix);
    case "staticlibrary":
        return staticLibraryFilePath(product, variantSuffix);
    case "dynamiclibrary":
        return dynamicLibraryFilePath(product, variantSuffix, version, maxParts);
    default:
        throw new Error("Unknown linker output file tag: " + fileTag);
    }
}

function importLibraryFilePath(product) {
    return product.moduleProperty("cpp", "dynamicLibraryPrefix")
         + product.targetName
         + (product.cpp.variantSuffix || "")
         + product.moduleProperty("cpp", "dynamicLibraryImportSuffix");
}

function debugInfoIsBundle(product) {
    if (!product.moduleProperty("qbs", "targetOS").contains("darwin"))
        return false;
    var flags = product.moduleProperty("cpp", "dsymutilFlags") || [];
    return !flags.contains("-f") && !flags.contains("--flat");
}

function debugInfoFileName(product, variantSuffix, fileTag) {
    var suffix = "";

    // For dSYM bundles, the DWARF debug info file has no suffix
    if (!product.moduleProperty("qbs", "targetOS").contains("darwin")
            || !debugInfoIsBundle(product))
        suffix = product.moduleProperty("cpp", "debugInfoSuffix");

    if (product.moduleProperty("bundle", "isBundle")) {
        return FileInfo.fileName(bundleExecutableFilePath(product, variantSuffix)) + suffix;
    } else {
        switch (fileTag) {
        case "application":
            return applicationFilePath(product, variantSuffix) + suffix;
        case "dynamiclibrary":
            return dynamicLibraryFilePath(product, variantSuffix) + suffix;
        case "loadablemodule":
            return loadableModuleFilePath(product, variantSuffix) + suffix;
        case "staticlibrary":
            return staticLibraryFilePath(product, variantSuffix) + suffix;
        default:
            return product.targetName + (variantSuffix || "") + suffix;
        }
    }
}

function debugInfoBundlePath(product, fileTag) {
    if (!debugInfoIsBundle(product))
        return undefined;
    var suffix = product.moduleProperty("cpp", "debugInfoBundleSuffix");
    if (product.moduleProperty("qbs", "targetOS").contains("darwin")
            && product.moduleProperty("bundle", "isBundle"))
        return product.moduleProperty("bundle", "bundleName") + suffix;
    return debugInfoFileName(product, undefined, fileTag) + suffix;
}

function debugInfoFilePath(product, variantSuffix, fileTag) {
    var name = debugInfoFileName(product, variantSuffix, fileTag);
    if (product.moduleProperty("qbs", "targetOS").contains("darwin") && debugInfoIsBundle(product)) {
        return FileInfo.joinPaths(debugInfoBundlePath(product, fileTag),
                                  "Contents", "Resources", "DWARF", name);
    } else if (product.moduleProperty("bundle", "isBundle")) {
        return FileInfo.joinPaths(product.moduleProperty("bundle", "executableFolderPath"), name);
    }

    return name;
}

function debugInfoPlistFilePath(product, fileTag) {
    if (!debugInfoIsBundle(product))
        return undefined;
    return FileInfo.joinPaths(debugInfoBundlePath(product, fileTag), "Contents", "Info.plist");
}

// Returns whether the string looks like a library filename
function isLibraryFileName(product, fileName, prefix, suffixes, isShared) {
    var suffix, i;
    var os = product.moduleProperty("qbs", "targetOS");
    for (i = 0; i < suffixes.length; ++i) {
        suffix = suffixes[i];
        if (isShared && os.contains("unix") && !os.contains("darwin"))
            suffix += "(\\.[0-9]+){0,3}";
        if (fileName.match("^" + prefix + ".+?\\" + suffix + "$"))
            return true;
    }
    return false;
}

function frameworkExecutablePath(frameworkPath) {
    var suffix = ".framework";
    var isAbsoluteFrameworkPath = frameworkPath.slice(-suffix.length) === suffix;
    if (isAbsoluteFrameworkPath) {
        var frameworkName = FileInfo.fileName(frameworkPath).slice(0, -suffix.length);
        return FileInfo.joinPaths(frameworkPath, frameworkName);
    }
    return undefined;
}

// pathList is also a string, using the respective separator
function prependOrSetPath(path, pathList, separator) {
    if (!pathList || pathList.length === 0)
        return path;
    return path + separator + pathList;
}
