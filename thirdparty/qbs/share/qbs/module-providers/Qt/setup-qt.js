/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

var Environment = require("qbs.Environment");
var File = require("qbs.File");
var FileInfo = require("qbs.FileInfo");
var Process = require("qbs.Process");
var TextFile = require("qbs.TextFile");
var Utilities = require("qbs.Utilities");

function splitNonEmpty(s, c) { return s.split(c).filter(function(e) { return e; }); }
function toNative(p) { return FileInfo.toNativeSeparators(p); }
function exeSuffix(qbs) { return qbs.hostOS.contains("windows") ? ".exe" : ""; }

function getQmakeFilePaths(qmakeFilePaths, qbs) {
    if (qmakeFilePaths && qmakeFilePaths.length > 0)
        return qmakeFilePaths;
    console.info("Detecting Qt installations...");
    var pathValue = Environment.getEnv("PATH");
    if (!pathValue)
        return [];
    var dirs = splitNonEmpty(pathValue, qbs.pathListSeparator);
    var suffix = exeSuffix(qbs);
    var filePaths = [];
    for (var i = 0; i < dirs.length; ++i) {
        var candidate = FileInfo.joinPaths(dirs[i], "qmake" + suffix);
        var canonicalCandidate = FileInfo.canonicalPath(candidate);
        if (!canonicalCandidate || !File.exists(canonicalCandidate))
            continue;
        if (FileInfo.completeBaseName(canonicalCandidate) !== "qtchooser")
            candidate = canonicalCandidate;
        if (!filePaths.contains(candidate)) {
            console.info("Found Qt at '" + toNative(candidate) + "'.");
            filePaths.push(candidate);
        }
    }
    return filePaths;
}

function queryQmake(qmakeFilePath) {
    var qmakeProcess = new Process;
    qmakeProcess.exec(qmakeFilePath, ["-query"]);
    if (qmakeProcess.exitCode() !== 0) {
        throw "The qmake executable '" + toNative(qmakeFilePath) + "' failed with exit code "
                + qmakeProcess.exitCode() + ".";
    }
    var queryResult = {};
    while (!qmakeProcess.atEnd()) {
        var line = qmakeProcess.readLine();
        var index = (line || "").indexOf(":");
        if (index !== -1)
            queryResult[line.slice(0, index)] = line.slice(index + 1).trim();
    }
    return queryResult;
}

function pathQueryValue(queryResult, key) {
    var p = queryResult[key];
    if (p)
        return FileInfo.fromNativeSeparators(p);
}

function readFileContent(filePath) {
    var f = new TextFile(filePath, TextFile.ReadOnly);
    var content = f.readAll();
    f.close();
    return content;
}

// TODO: Don't do the split every time...
function configVariable(configContent, key) {
    var configContentLines = configContent.split('\n');
    var regexp = new RegExp("\\s*" + key + "\\s*\\+{0,1}=(.*)");
    for (var i = 0; i < configContentLines.length; ++i) {
        var line = configContentLines[i];
        var match = regexp.exec(line);
        if (match)
            return match[1].trim();
    }
}

function configVariableItems(configContent, key) {
    return splitNonEmpty(configVariable(configContent, key), ' ');
}

function msvcPrefix() { return "win32-msvc"; }

function isMsvcQt(qtProps) { return qtProps.mkspecName.startsWith(msvcPrefix()); }

function msvcCompilerVersionForYear(year) {
    var mapping = {
        "2005": "14", "2008": "15", "2010": "16", "2012": "17", "2013": "18", "2015": "19",
        "2017": "19.1"
    };
    return mapping[year];
}

function msvcCompilerVersionFromMkspecName(mkspecName) {
    return msvcCompilerVersionForYear(mkspecName.slice(msvcPrefix().length));
}

function addQtBuildVariant(qtProps, buildVariantName) {
    if (qtProps.qtConfigItems.contains(buildVariantName))
        qtProps.buildVariant.push(buildVariantName);
}

function checkForStaticBuild(qtProps) {
    if (qtProps.qtMajorVersion >= 5)
        return qtProps.qtConfigItems.contains("static");
    if (qtProps.frameworkBuild)
        return false; // there are no Qt4 static frameworks
    var isWin = qtProps.mkspecName.startsWith("win");
    var libDir = isWin ? qtProps.binaryPath : qtProps.libraryPath;
    var coreLibFiles = File.directoryEntries(libDir, File.Files)
        .filter(function(fp) { return fp.contains("Core"); });
    if (coreLibFiles.length === 0)
        throw "Could not determine whether Qt is a static build.";
    for (var i = 0; i < coreLibFiles.length; ++i) {
        if (Utilities.isSharedLibrary(coreLibFiles[i]))
            return false;
    }
    return true;
}

function isForMinGw(qtProps) {
    return qtProps.mkspecName.startsWith("win32-g++") || qtProps.mkspecName.startsWith("mingw");
}

function targetsDesktopWindows(qtProps) {
    return qtProps.mkspecName.startsWith("win32-") || isForMinGw(qtProps);
}

function guessMinimumWindowsVersion(qtProps) {
    if (qtProps.mkspecName.startsWith("winrt-"))
        return "10.0";
    if (!targetsDesktopWindows(qtProps))
        return "";
    if (qtProps.architecture === "x86_64" || qtProps.architecture === "ia64")
        return "5.2"
    var match = qtProps.mkspecName.match(/^win32-msvc(\d+)$/);
    if (match) {
        var msvcVersion = match[1];
        if (msvcVersion < 2012)
            return "5.0";
        return "5.1";
    }
    return qtProps.qtMajorVersion < 5 ? "5.0" : "5.1";
}

function fillEntryPointLibs(qtProps, debug) {
    result = [];
    var isMinGW = isForMinGw(qtProps);

    // Some Linux distributions rename the qtmain library.
    var qtMainCandidates = ["qtmain"];
    if (isMinGW && qtProps.qtMajorVersion === 5)
        qtMainCandidates.push("qt5main");

    for (var i = 0; i < qtMainCandidates.length; ++i) {
        var baseNameCandidate = qtMainCandidates[i];
        var qtmain = qtProps.libraryPath + '/';
        if (isMinGW)
            qtmain += "lib";
        qtmain += baseNameCandidate + qtProps.qtLibInfix;
        if (debug)
            qtmain += 'd';
        if (isMinGW) {
            qtmain += ".a";
        } else {
            qtmain += ".lib";
            if (Utilities.versionCompare(qtProps.qtVersion, "5.4.0") >= 0)
                result.push("Shell32.lib");
        }
        if (File.exists(qtmain)) {
            result.push(qtmain);
            break;
        }
    }
    if (result.length === 0) {
        console.warn("Could not find the qtmain library at '" + toNative(qtProps.libraryPath)
                     + "'. You will not be able to link Qt applications.");
    }
    return result;
}

function getQtProperties(qmakeFilePath, qbs) {
    var queryResult = queryQmake(qmakeFilePath);
    var qtProps = {};
    qtProps.installPrefixPath = pathQueryValue(queryResult, "QT_INSTALL_PREFIX");
    qtProps.documentationPath = pathQueryValue(queryResult, "QT_INSTALL_DOCS");
    qtProps.includePath = pathQueryValue(queryResult, "QT_INSTALL_HEADERS");
    qtProps.libraryPath = pathQueryValue(queryResult, "QT_INSTALL_LIBS");
    qtProps.binaryPath = pathQueryValue(queryResult, "QT_HOST_BINS")
            || pathQueryValue(queryResult, "QT_INSTALL_BINS");
    qtProps.documentationPath = pathQueryValue(queryResult, "QT_INSTALL_DOCS");
    qtProps.pluginPath = pathQueryValue(queryResult, "QT_INSTALL_PLUGINS");
    qtProps.qmlPath = pathQueryValue(queryResult, "QT_INSTALL_QML");
    qtProps.qmlImportPath = pathQueryValue(queryResult, "QT_INSTALL_IMPORTS");
    qtProps.qtVersion = queryResult.QT_VERSION;

    var mkspecsBaseSrcPath;
    if (Utilities.versionCompare(qtProps.qtVersion, "5") >= 0) {
        qtProps.mkspecBasePath = FileInfo.joinPaths(pathQueryValue(queryResult, "QT_HOST_DATA"),
                                                    "mkspecs");
        mkspecsBaseSrcPath = FileInfo.joinPaths(pathQueryValue(queryResult, "QT_HOST_DATA/src"),
                                                "mkspecs");
    } else {
        qtProps.mkspecBasePath = FileInfo.joinPaths
                (pathQueryValue(queryResult, "QT_INSTALL_DATA"), "mkspecs");
    }
    if (!File.exists(qtProps.mkspecBasePath))
        throw "Cannot extract the mkspecs directory.";

    var qconfigContent = readFileContent(FileInfo.joinPaths(qtProps.mkspecBasePath,
                                                            "qconfig.pri"));
    qtProps.qtMajorVersion = parseInt(configVariable(qconfigContent, "QT_MAJOR_VERSION"));
    qtProps.qtMinorVersion = parseInt(configVariable(qconfigContent, "QT_MINOR_VERSION"));
    qtProps.qtPatchVersion = parseInt(configVariable(qconfigContent, "QT_PATCH_VERSION"));
    qtProps.qtNameSpace = configVariable(qconfigContent, "QT_NAMESPACE");
    qtProps.qtLibInfix = configVariable(qconfigContent, "QT_LIBINFIX") || "";
    qtProps.architecture = configVariable(qconfigContent, "QT_TARGET_ARCH")
            || configVariable(qconfigContent, "QT_ARCH") || "x86";
    qtProps.configItems = configVariableItems(qconfigContent, "CONFIG");
    qtProps.qtConfigItems = configVariableItems(qconfigContent, "QT_CONFIG");

    // retrieve the mkspec
    if (qtProps.qtMajorVersion >= 5) {
        qtProps.mkspecName = queryResult.QMAKE_XSPEC;
        qtProps.mkspecPath = FileInfo.joinPaths(qtProps.mkspecBasePath, qtProps.mkspecName);
        if (mkspecsBaseSrcPath && !File.exists(qtProps.mkspecPath))
            qtProps.mkspecPath = FileInfo.joinPaths(mkspecsBaseSrcPath, qtProps.mkspecName);
    } else {
        if (qbs.hostOS.contains("windows")) {
            var baseDirPath = FileInfo.joinPaths(qtProps.mkspecBasePath, "default");
            var fileContent = readFileContent(FileInfo.joinPaths(baseDirPath, "qmake.conf"));
            qtProps.mkspecPath = configVariable(fileContent, "QMAKESPEC_ORIGINAL");
            if (!File.exists(qtProps.mkspecPath)) {
                // Work around QTBUG-28792.
                // The value of QMAKESPEC_ORIGINAL is wrong for MinGW packages. Y u h8 me?
                var match = fileContent.exec(/\binclude\(([^)]+)\/qmake\.conf\)/m);
                if (match) {
                    qtProps.mkspecPath = FileInfo.cleanPath(FileInfo.joinPaths(
                                                                   baseDirPath, match[1]));
                }
            }
        } else {
            qtProps.mkspecPath = FileInfo.canonicalPath(
                        FileInfo.joinPaths(qtProps.mkspecBasePath, "default"));
        }

        // E.g. in qmake.conf for Qt 4.8/mingw we find this gem:
        //    QMAKESPEC_ORIGINAL=C:\\Qt\\Qt\\4.8\\mingw482\\mkspecs\\win32-g++
        qtProps.mkspecPath = FileInfo.cleanPath(qtProps.mkspecPath);

        qtProps.mkspecName = qtProps.mkspecPath;
        var idx = qtProps.mkspecName.lastIndexOf('/');
        if (idx !== -1)
            qtProps.mkspecName = qtProps.mkspecName.slice(idx + 1);
    }
    if (!File.exists(qtProps.mkspecPath))
        throw "mkspec '" + toNative(qtProps.mkspecPath) + "' does not exist";

    // determine MSVC version
    if (isMsvcQt(qtProps)) {
        var msvcMajor = configVariable(qconfigContent, "QT_MSVC_MAJOR_VERSION");
        var msvcMinor = configVariable(qconfigContent, "QT_MSVC_MINOR_VERSION");
        var msvcPatch = configVariable(qconfigContent, "QT_MSVC_PATCH_VERSION");
        if (msvcMajor && msvcMinor && msvcPatch)
            qtProps.msvcVersion = msvcMajor + "." + msvcMinor + "." + msvcPatch;
        else
            qtProps.msvcVersion = msvcCompilerVersionFromMkspecName(qtProps.mkspecName);
    }

    // determine whether we have a framework build
    qtProps.frameworkBuild = qtProps.mkspecPath.contains("macx")
            && qtProps.configItems.contains("qt_framework");

    // determine whether Qt is built with debug, release or both
    qtProps.buildVariant = [];
    addQtBuildVariant(qtProps, "debug");
    addQtBuildVariant(qtProps, "release");

    qtProps.staticBuild = checkForStaticBuild(qtProps);

    // determine whether user apps require C++11
    if (qtProps.qtConfigItems.contains("c++11") && qtProps.staticBuild)
        qtProps.configItems.push("c++11");

    // Set the minimum operating system versions appropriate for this Qt version
    qtProps.windowsVersion = guessMinimumWindowsVersion(qtProps);
    if (qtProps.windowsVersion) {    // Is target OS Windows?
        if (qtProps.buildVariant.contains("debug"))
            qtProps.entryPointLibsDebug = fillEntryPointLibs(qtProps, true);
        if (qtProps.buildVariant.contains("release"))
            qtProps.entryPointLibsRelease = fillEntryPointLibs(qtProps, false);
    } else if (qtProps.mkspecPath.contains("macx")) {
        if (qtProps.qtMajorVersion >= 5) {
            var lines = getFileContentsRecursively(FileInfo.joinPaths(qtProps.mkspecPath,
                                                                      "qmake.conf"));
            for (var i = 0; i < lines.length; ++i) {
                var line = lines[i].trim();
                match = line.match
                        (/^QMAKE_(MACOSX|IOS|TVOS|WATCHOS)_DEPLOYMENT_TARGET\s*=\s*(.*)\s*$/);
                if (match) {
                    var platform = match[1];
                    var version = match[2];
                    if (platform === "MACOSX")
                        qtProps.macosVersion = version;
                    else if (platform === "IOS")
                        qtProps.iosVersion = version;
                    else if (platform === "TVOS")
                        qtProps.tvosVersion = version;
                    else if (platform === "WATCHOS")
                        qtProps.watchosVersion = version;
                }
            }
            var isMac = qtProps.mkspecName !== "macx-ios-clang"
                    && qtProps.mkspecName !== "macx-tvos-clang"
                    && qtProps.mkspecName !== "macx-watchos-clang";
            if (isMac) {
                // Qt 5.0.x placed the minimum version in a different file
                if (!qtProps.macosVersion)
                    qtProps.macosVersion = "10.6";

                // If we're using C++11 with libc++, make sure the deployment target is >= 10.7
                if (Utilities.versionCompare(qtProps.macosVersion, "10, 7") < 0
                        && qtProps.qtConfigItems.contains("c++11")) {
                    qtProps.macosVersion = "10.7";
                }
            }
        } else if (qtProps.qtMajorVersion === 4 && qtProps.qtMinorVersion >= 6) {
            var qconfigDir = qtProps.frameworkBuild
                    ? FileInfo.joinPaths(qtProps.libraryPath, "QtCore.framework", "Headers")
                    : FileInfo.joinPaths(qtProps.includePath, "Qt");
            try {
                var qconfig = new TextFile(FileInfo.joinPaths(qconfigDir, "qconfig.h"),
                                           TextFile.ReadOnly);
                var qtCocoaBuild = false;
                var ok = true;
                do {
                    line = qconfig.readLine();
                    if (line.match(/\s*#define\s+QT_MAC_USE_COCOA\s+1\s*/)) {
                        qtCocoaBuild = true;
                        break;
                    }
                } while (!qconfig.atEof());
                qtProps.macosVersion = qtCocoaBuild ? "10.5" : "10.4";
            }
            catch (e) {}
            finally {
                if (qconfig)
                    qconfig.close();
            }
            if (!qtProps.macosVersion) {
                throw "Could not determine whether Qt is using Cocoa or Carbon from '"
                        + toNative(qconfig.filePath()) + "'.";
            }
        }
    } else if (qtProps.mkspecPath.contains("android")) {
        if (qtProps.qtMajorVersion >= 5)
            qtProps.androidVersion = "2.3";
        else if (qtProps.qtMajorVersion === 4 && qtProps.qtMinorVersion >= 8)
            qtProps.androidVersion = "1.6"; // Necessitas
    }
    return qtProps;
}

function makePluginData() {
    var pluginData = {};
    pluginData.type = undefined;
    pluginData.className = undefined;
    pluginData.autoLoad = true;
    pluginData["extends"] = [];
    return pluginData;
}

function makeQtModuleInfo(name, qbsName, deps) {
    var moduleInfo = {};
    moduleInfo.name = name; // As in the path to the headers and ".name" in the pri files.
    if (moduleInfo.name === undefined)
        moduleInfo.name = "";
    moduleInfo.qbsName = qbsName; // Lower-case version without "qt" prefix.
    moduleInfo.dependencies = deps || []; // qbs names.
    if (moduleInfo.qbsName !== "core" && !moduleInfo.dependencies.contains("core"))
        moduleInfo.dependencies.unshift("core");
    moduleInfo.isPrivate = qbsName && qbsName.endsWith("-private");
    moduleInfo.hasLibrary = !moduleInfo.isPrivate;
    moduleInfo.isStaticLibrary = false;
    moduleInfo.isPlugin = false;
    moduleInfo.mustExist = true;
    moduleInfo.modulePrefix = ""; // empty value means "Qt".
    moduleInfo.version = undefined;
    moduleInfo.includePaths = [];
    moduleInfo.compilerDefines = [];
    moduleInfo.staticLibrariesDebug = [];
    moduleInfo.staticLibrariesRelease = [];
    moduleInfo.dynamicLibrariesDebug = [];
    moduleInfo.dynamicLibrariesRelease = [];
    moduleInfo.linkerFlagsDebug = [];
    moduleInfo.linkerFlagsRelease = [];
    moduleInfo.libFilePathDebug = undefined;
    moduleInfo.libFilePathRelease = undefined;
    moduleInfo.frameworksDebug = [];
    moduleInfo.frameworksRelease = [];
    moduleInfo.frameworkPathsDebug = [];
    moduleInfo.frameworkPathsRelease = [];
    moduleInfo.libraryPaths = [];
    moduleInfo.config = [];
    moduleInfo.supportedPluginTypes = [];
    moduleInfo.pluginData = makePluginData();
    return moduleInfo;
}

function frameworkHeadersPath(qtModuleInfo, qtProps) {
    return FileInfo.joinPaths(qtProps.libraryPath, qtModuleInfo.name + ".framework", "Headers");
}

function qt4ModuleIncludePaths(qtModuleInfo, qtProps) {
    var paths = [];
    if (isFramework(qtModuleInfo, qtProps))
        paths.push(frameworkHeadersPath(qtModuleInfo, qtProps));
    else
        paths.push(qtProps.includePath, FileInfo.joinPaths(qtProps.includePath, qtModuleInfo.name));
    return paths;
}

// We erroneously called the "testlib" module "test" for quite a while. Let's not punish users
// for that.
function addTestModule(modules) {
    var testModule = makeQtModuleInfo("QtTest", "test", ["testlib"]);
    testModule.hasLibrary = false;
    modules.push(testModule);
}

// See above.
function addDesignerComponentsModule(modules) {
    var module = makeQtModuleInfo("QtDesignerComponents", "designercomponents",
                                  ["designercomponents-private"]);
    module.hasLibrary = false;
    modules.push(module);
}

function isFramework(modInfo, qtProps) {
    if (!qtProps.frameworkBuild || modInfo.isStaticLibrary)
        return false;
    var modulesNeverBuiltAsFrameworks = [
        "bootstrap", "openglextensions", "platformsupport", "qmldevtools", "uitools", "harfbuzzng"
    ];
    return !modulesNeverBuiltAsFrameworks.contains(modInfo.qbsName);
}

function libBaseName(modInfo, libName, debugBuild, qtProps) {
    var name = libName;
    if (qtProps.mkspecName.startsWith("win")) {
        if (debugBuild)
            name += 'd';
        if (!modInfo.isStaticLibrary && qtProps.qtMajorVersion < 5)
            name += qtProps.qtMajorVersion;
    }
    if (qtProps.mkspecName.contains("macx")
            || qtProps.mkspecName.contains("ios")
            || qtProps.mkspecName.contains("darwin")) {
        if (!isFramework(modInfo, qtProps)
                && qtProps.buildVariant.contains("debug")
                && (!qtProps.buildVariant.contains("release") || debugBuild)) {
            name += "_debug";
        }
    }
    return name;
}

function moduleNameWithoutPrefix(modInfo) {
    if (modInfo.name === "Phonon")
        return "phonon";
    if (!modInfo.modulePrefix && modInfo.name.startsWith("Qt"))
        return modInfo.name.slice(2); // Strip off "Qt".
    if (modInfo.name.startsWith(modInfo.modulePrefix))
        return modInfo.name.slice(modInfo.modulePrefix.length);
    return modInfo.name;
}

function libraryBaseName(modInfo, qtProps, debugBuild) {
    if (modInfo.isPlugin)
        return libBaseName(modInfo, modInfo.name, debugBuild, qtProps);

    // Some modules use a different naming scheme, so it doesn't get boring.
    var libNameBroken = modInfo.name === "Enginio"
            || modInfo.name === "DataVisualization"
            || modInfo.name === "Phonon";

    var libName = !modInfo.modulePrefix && !libNameBroken ? "Qt" : modInfo.modulePrefix;
    if (qtProps.qtMajorVersion >= 5 && !isFramework(modInfo, qtProps) && !libNameBroken)
        libName += qtProps.qtMajorVersion;
    libName += moduleNameWithoutPrefix(modInfo);
    libName += qtProps.qtLibInfix;
    return libBaseName(modInfo, libName, debugBuild, qtProps);
}

function libNameForLinker(modInfo, qtProps, debugBuild) {
    if (!modInfo.hasLibrary)
        return undefined;
    var libName = libraryBaseName(modInfo, qtProps, debugBuild);
    if (qtProps.mkspecName.contains("msvc"))
        libName += ".lib";
    return libName;
}

function guessLibraryFilePath(prlFilePath, libDir, qtProps) {
    var baseName = FileInfo.baseName(prlFilePath);
    var prefixCandidates = ["", "lib"];
    var suffixCandidates = ["so." + qtProps.qtVersion, "so", "a", "lib", "dll.a"];
    for (var i = 0; i < prefixCandidates.length; ++i) {
        var prefix = prefixCandidates[i];
        for (var j = 0; j < suffixCandidates.length; ++j) {
            var suffix = suffixCandidates[j];
            var candidate = FileInfo.joinPaths(libDir, prefix + baseName + '.' + suffix);
            if (File.exists(candidate))
                return candidate;
        }
    }
}

function doReplaceQtLibNamesWithFilePath(namePathMap, libList) {
    for (var i = 0; i < libList.length; ++i) {
        var lib = libList[i];
        var path = namePathMap[lib];
        if (path)
            libList[i] = path;
    }
}

function replaceQtLibNamesWithFilePath(modules, qtProps) {
    // We don't want to add the libraries for Qt modules via "-l", because of the
    // danger that a wrong one will be picked up, e.g. from /usr/lib. Instead,
    // we pull them in using the full file path.
    var linkerNamesToFilePathsDebug = {};
    var linkerNamesToFilePathsRelease = {};
    for (var i = 0; i < modules.length; ++i) {
        var m = modules[i];
        linkerNamesToFilePathsDebug[libNameForLinker(m, qtProps, true)] = m.libFilePathDebug;
        linkerNamesToFilePathsRelease[libNameForLinker(m, qtProps, false)] = m.libFilePathRelease;
    }
    for (i = 0; i < modules.length; ++i) {
        var module = modules[i];
        doReplaceQtLibNamesWithFilePath(linkerNamesToFilePathsDebug, module.dynamicLibrariesDebug);
        doReplaceQtLibNamesWithFilePath(linkerNamesToFilePathsDebug, module.staticLibrariesDebug);
        doReplaceQtLibNamesWithFilePath(linkerNamesToFilePathsRelease,
                                        module.dynamicLibrariesRelease);
        doReplaceQtLibNamesWithFilePath(linkerNamesToFilePathsRelease,
                                        module.staticLibrariesRelease);
    }
}

function doSetupLibraries(modInfo, qtProps, debugBuild, nonExistingPrlFiles) {
    if (!modInfo.hasLibrary)
        return; // Can happen for Qt4 convenience modules, like "widgets".

    if (debugBuild) {
        if (!qtProps.buildVariant.contains("debug"))
            return;
        var modulesNeverBuiltAsDebug = ["bootstrap", "qmldevtools"];
        for (var i = 0; i < modulesNeverBuiltAsDebug.length; ++i) {
            var m = modulesNeverBuiltAsDebug[i];
            if (modInfo.qbsName === m || modInfo.qbsName === m + "-private")
                return;
        }
    } else if (!qtProps.buildVariant.contains("release")) {
        return;
    }

    var libs = modInfo.isStaticLibrary
            ? (debugBuild ? modInfo.staticLibrariesDebug : modInfo.staticLibrariesRelease)
            : (debugBuild ? modInfo.dynamicLibrariesDebug : modInfo.dynamicLibrariesRelease);
    var frameworks = debugBuild ? modInfo.frameworksDebug : modInfo.frameworksRelease;
    var frameworkPaths = debugBuild ? modInfo.frameworkPathsDebug : modInfo.frameworkPathsRelease;
    var flags = debugBuild ? modInfo.linkerFlagsDebug : modInfo.linkerFlagsRelease;
    var libFilePath;

    if (qtProps.mkspecName.contains("ios") && modInfo.isStaticLibrary) {
        libs.push("z", "m");
        if (qtProps.qtMajorVersion === 5 && qtProps.qtMinorVersion < 8) {
            var platformSupportModule = makeQtModuleInfo("QtPlatformSupport", "platformsupport");
            libs.push(libNameForLinker(platformSupportModule, qtProps, debugBuild));
        }
        if (modInfo.name === "qios") {
            flags.push("-force_load", FileInfo.joinPaths(
                           qtProps.pluginPath, "platforms",
                           libBaseName(modInfo, "libqios", debugBuild, qtProps) + ".a"));
        }
    }
    var prlFilePath = modInfo.isPlugin
            ? FileInfo.joinPaths(qtProps.pluginPath, modInfo.pluginData.type)
            : qtProps.libraryPath;
    if (isFramework(modInfo, qtProps)) {
        prlFilePath = FileInfo.joinPaths(prlFilePath,
                                         libraryBaseName(modInfo, qtProps, false) + ".framework");
    }
    var libDir = prlFilePath;
    var baseName = libraryBaseName(modInfo, qtProps, debugBuild);
    if (!qtProps.mkspecName.startsWith("win") && !isFramework(modInfo, qtProps))
        baseName = "lib" + baseName;
    prlFilePath = FileInfo.joinPaths(prlFilePath, baseName);
    var isNonStaticQt4OnWindows = qtProps.mkspecName.startsWith("win")
            && !modInfo.isStaticLibrary && qtProps.qtMajorVersion < 5;
    if (isNonStaticQt4OnWindows)
        prlFilePath = prlFilePath.slice(0, prlFilePath.length - 1); // The prl file base name does *not* contain the version number...
    prlFilePath += ".prl";
    try {
        var prlFile = new TextFile(prlFilePath, TextFile.ReadOnly);
        while (!prlFile.atEof()) {
            var line = prlFile.readLine().trim();
            var equalsOffset = line.indexOf('=');
            if (equalsOffset === -1)
                continue;
            if (line.startsWith("QMAKE_PRL_TARGET")) {
                var isMingw = qtProps.mkspecName.startsWith("win")
                        && qtProps.mkspecName.contains("g++");
                var isQtVersionBefore56 = qtProps.qtMajorVersion < 5
                        || (qtProps.qtMajorVersion === 5 && qtProps.qtMinorVersion < 6);

                // QMAKE_PRL_TARGET has a "lib" prefix, except for mingw.
                // Of course, the exception has an exception too: For static libs, mingw *does*
                // have the "lib" prefix.
                var libFileName = "";
                if (isQtVersionBefore56 && qtProps.qtMajorVersion === 5 && isMingw
                        && !modInfo.isStaticLibrary) {
                    libFileName += "lib";
                }

                libFileName += line.slice(equalsOffset + 1).trim();
                if (isNonStaticQt4OnWindows)
                    libFileName += 4; // This is *not* part of QMAKE_PRL_TARGET...
                if (isQtVersionBefore56) {
                    if (qtProps.mkspecName.contains("msvc")) {
                        libFileName += ".lib";
                    } else if (isMingw) {
                        libFileName += ".a";
                        if (!File.exists(FileInfo.joinPaths(libDir, libFileName)))
                            libFileName = libFileName.slice(0, -2) + ".dll";
                    }
                }
                libFilePath = FileInfo.joinPaths(libDir, libFileName);
                continue;
            }
            if (line.startsWith("QMAKE_PRL_CONFIG")) {
                modInfo.config = splitNonEmpty(line.slice(equalsOffset + 1).trim(), ' ');
                continue;
            }
            if (!line.startsWith("QMAKE_PRL_LIBS"))
                continue;

            var parts = extractPaths(line.slice(equalsOffset + 1).trim(), prlFilePath);
            for (i = 0; i < parts.length; ++i) {
                var part = parts[i];
                part = part.replace("$$[QT_INSTALL_LIBS]", qtProps.libraryPath);
                if (part.startsWith("-l")) {
                    libs.push(part.slice(2));
                } else if (part.startsWith("-L")) {
                    modInfo.libraryPaths.push(part.slice(2));
                } else if (part.startsWith("-F")) {
                    frameworkPaths.push(part.slice(2));
                } else if (part === "-framework") {
                    if (++i < parts.length)
                        frameworks.push(parts[i]);
                } else if (part === "-pthread") {
                    libs.push("pthread");
                } else if (part.startsWith('-')) { // Some other option
                    console.debug("QMAKE_PRL_LIBS contains non-library option '" + part
                                  + "' in file '" + prlFilePath + "'");
                    flags.push(part);
                } else if (part.startsWith("/LIBPATH:")) {
                    libraryPaths.push(part.slice(9).replace(/\\/g, '/'));
                } else { // Assume it's a file path/name.
                    libs.push(part.replace(/\\/g, '/'));
                }
            }
        }
    } catch (e) {
        libFilePath = guessLibraryFilePath(prlFilePath, libDir, qtProps);
        if (nonExistingPrlFiles.contains(prlFilePath))
            return;
        nonExistingPrlFiles.push(prlFilePath);
        if (!libFilePath && modInfo.mustExist) {
            console.warn("Could not open prl file '" + toNative(prlFilePath) + "' for module '"
                         + modInfo.name
                         + "' (" + e + "), and failed to deduce the library file path. "
                         + " This module will likely not be usable by qbs.");
        }
    }
    finally {
        if (prlFile)
            prlFile.close();
    }

    if (debugBuild)
        modInfo.libFilePathDebug = libFilePath;
    else
        modInfo.libFilePathRelease = libFilePath;
}

function setupLibraries(qtModuleInfo, qtProps, nonExistingPrlFiles) {
    doSetupLibraries(qtModuleInfo, qtProps, true, nonExistingPrlFiles);
    doSetupLibraries(qtModuleInfo, qtProps, false, nonExistingPrlFiles);
}

function allQt4Modules(qtProps) {
    // as per http://doc.qt.io/qt-4.8/modules.html + private stuff.
    var modules = [];

    var core = makeQtModuleInfo("QtCore", "core");
    core.compilerDefines.push("QT_CORE_LIB");
    if (qtProps.qtNameSpace)
        core.compilerDefines.push("QT_NAMESPACE=" + qtProps.qtNameSpace);
    modules.push(core,
                 makeQtModuleInfo("QtCore", "core-private", ["core"]),
                 makeQtModuleInfo("QtGui", "gui"),
                 makeQtModuleInfo("QtGui", "gui-private", ["gui"]),
                 makeQtModuleInfo("QtMultimedia", "multimedia", ["gui", "network"]),
                 makeQtModuleInfo("QtMultimedia", "multimedia-private", ["multimedia"]),
                 makeQtModuleInfo("QtNetwork", "network"),
                 makeQtModuleInfo("QtNetwork", "network-private", ["network"]),
                 makeQtModuleInfo("QtOpenGL", "opengl", ["gui"]),
                 makeQtModuleInfo("QtOpenGL", "opengl-private", ["opengl"]),
                 makeQtModuleInfo("QtOpenVG", "openvg", ["gui"]),
                 makeQtModuleInfo("QtScript", "script"),
                 makeQtModuleInfo("QtScript", "script-private", ["script"]),
                 makeQtModuleInfo("QtScriptTools", "scripttools", ["script", "gui"]),
                 makeQtModuleInfo("QtScriptTools", "scripttools-private", ["scripttools"]),
                 makeQtModuleInfo("QtSql", "sql"),
                 makeQtModuleInfo("QtSql", "sql-private", ["sql"]),
                 makeQtModuleInfo("QtSvg", "svg", ["gui"]),
                 makeQtModuleInfo("QtSvg", "svg-private", ["svg"]),
                 makeQtModuleInfo("QtWebKit", "webkit", ["gui", "network"]),
                 makeQtModuleInfo("QtWebKit", "webkit-private", ["webkit"]),
                 makeQtModuleInfo("QtXml", "xml"),
                 makeQtModuleInfo("QtXml", "xml-private", ["xml"]),
                 makeQtModuleInfo("QtXmlPatterns", "xmlpatterns", ["network"]),
                 makeQtModuleInfo("QtXmlPatterns", "xmlpatterns-private", ["xmlpatterns"]),
                 makeQtModuleInfo("QtDeclarative", "declarative", ["gui", "script"]),
                 makeQtModuleInfo("QtDeclarative", "declarative-private", ["declarative"]),
                 makeQtModuleInfo("QtDesigner", "designer", ["gui", "xml"]),
                 makeQtModuleInfo("QtDesigner", "designer-private", ["designer"]),
                 makeQtModuleInfo("QtUiTools", "uitools"),
                 makeQtModuleInfo("QtUiTools", "uitools-private", ["uitools"]),
                 makeQtModuleInfo("QtHelp", "help", ["network", "sql"]),
                 makeQtModuleInfo("QtHelp", "help-private", ["help"]),
                 makeQtModuleInfo("QtTest", "testlib"),
                 makeQtModuleInfo("QtTest", "testlib-private", ["testlib"]));
    if (qtProps.mkspecName.startsWith("win")) {
        var axcontainer = makeQtModuleInfo("QAxContainer", "axcontainer");
        axcontainer.modulePrefix = "Q";
        axcontainer.isStaticLibrary = true;
        axcontainer.includePaths.push(FileInfo.joinPaths(qtProps.includePath, "ActiveQt"));
        modules.push(axcontainer);

        var axserver = makeQtModuleInfo("QAxServer", "axserver");
        axserver.modulePrefix = "Q";
        axserver.isStaticLibrary = true;
        axserver.compilerDefines.push("QAXSERVER");
        axserver.includePaths.push(FileInfo.joinPaths(qtProps.includePath, "ActiveQt"));
        modules.push(axserver);
    } else {
        modules.push(makeQtModuleInfo("QtDBus", "dbus"));
        modules.push(makeQtModuleInfo("QtDBus", "dbus-private", ["dbus"]));
    }

    var designerComponentsPrivate = makeQtModuleInfo(
                "QtDesignerComponents", "designercomponents-private",
                ["gui-private", "designer-private"]);
    designerComponentsPrivate.hasLibrary = true;
    modules.push(designerComponentsPrivate);

    var phonon = makeQtModuleInfo("Phonon", "phonon");
    phonon.includePaths = qt4ModuleIncludePaths(phonon, qtProps);
    modules.push(phonon);

    // Set up include paths that haven't been set up before this point.
    for (i = 0; i < modules.length; ++i) {
        var module = modules[i];
        if (module.includePaths.length > 0)
            continue;
        module.includePaths = qt4ModuleIncludePaths(module, qtProps);
    }

    // Set up compiler defines haven't been set up before this point.
    for (i = 0; i < modules.length; ++i) {
        module = modules[i];
        if (module.compilerDefines.length > 0)
            continue;
        module.compilerDefines.push("QT_" + module.qbsName.toUpperCase() + "_LIB");
    }

    // These are for the convenience of project file authors. It allows them
    // to add a dependency to e.g. "Qt.widgets" without a version check.
    var virtualModule = makeQtModuleInfo(undefined, "widgets", ["core", "gui"]);
    virtualModule.hasLibrary = false;
    modules.push(virtualModule);
    virtualModule = makeQtModuleInfo(undefined, "quick", ["declarative"]);
    virtualModule.hasLibrary = false;
    modules.push(virtualModule);
    virtualModule = makeQtModuleInfo(undefined, "concurrent");
    virtualModule.hasLibrary = false;
    modules.push(virtualModule);
    virtualModule = makeQtModuleInfo(undefined, "printsupport", ["core", "gui"]);
    virtualModule.hasLibrary = false;
    modules.push(virtualModule);

    addTestModule(modules);
    addDesignerComponentsModule(modules);

    var modulesThatCanBeDisabled = [
                "xmlpatterns", "multimedia", "phonon", "svg", "webkit", "script", "scripttools",
                "declarative", "gui", "dbus", "opengl", "openvg"];
    var nonExistingPrlFiles = [];
    for (i = 0; i < modules.length; ++i) {
        module = modules[i];
        var name = module.qbsName;
        var privateIndex = name.indexOf("-private");
        if (privateIndex !== -1)
            name = name.slice(0, privateIndex);
        if (modulesThatCanBeDisabled.contains(name))
            module.mustExist = false;
        if (qtProps.staticBuild)
            module.isStaticLibrary = true;
        setupLibraries(module, qtProps, nonExistingPrlFiles);
    }
    replaceQtLibNamesWithFilePath(modules, qtProps);

    return modules;
}

function getFileContentsRecursively(filePath) {
    var file = new TextFile(filePath, TextFile.ReadOnly);
    var lines = splitNonEmpty(file.readAll(), '\n');
    for (var i = 0; i < lines.length; ++i) {
        var includeString = "include(";
        var line = lines[i].trim();
        if (!line.startsWith(includeString))
            continue;
        var offset = includeString.length;
        var closingParenPos = line.indexOf(')', offset);
        if (closingParenPos === -1) {
            console.warn("Invalid include statement in '" + toNative(filePath) + "'");
            continue;
        }
        var includedFilePath = line.slice(offset, closingParenPos);
        if (!FileInfo.isAbsolutePath(includedFilePath))
            includedFilePath = FileInfo.joinPaths(FileInfo.path(filePath), includedFilePath);
        var includedContents = getFileContentsRecursively(includedFilePath);
        var j = i;
        for (var k = 0; k < includedContents.length; ++k)
            lines.splice(++j, 0, includedContents[k]);
        lines.splice(i--, 1);
    }
    file.close();
    return lines;
}

function extractPaths(rhs, filePath) {
    var paths = [];
    var startIndex = 0;
    for (;;) {
        while (startIndex < rhs.length && rhs.charAt(startIndex) === ' ')
            ++startIndex;
        if (startIndex >= rhs.length)
            break;
        var endIndex;
        if (rhs.charAt(startIndex) === '"') {
            ++startIndex;
            endIndex = rhs.indexOf('"', startIndex);
            if (endIndex === -1) {
                console.warn("Unmatched quote in file '" + toNative(filePath) + "'");
                break;
            }
        } else {
            endIndex = rhs.indexOf(' ', startIndex + 1);
            if (endIndex === -1)
                endIndex = rhs.length;
        }
        paths.push(FileInfo.cleanPath(rhs.slice(startIndex, endIndex)));
        startIndex = endIndex + 1;
    }
    return paths;
}

function removeDuplicatedDependencyLibs(modules) {
    var revDeps = {};
    var currentPath;
    var getLibraries;
    var getLibFilePath;

    function setupReverseDependencies(modules) {
        var moduleByName = {};
        for (var i = 0; i < modules.length; ++i)
            moduleByName[modules[i].qbsName] = modules[i];
        for (i = 0; i < modules.length; ++i) {
            var module = modules[i];
            for (var j = 0; j < module.dependencies.length; ++j) {
                var depmod = moduleByName[module.dependencies[j]];
                if (!depmod)
                    continue;
                if (!revDeps[depmod])
                    revDeps[depmod] = [];
                revDeps[depmod].push(module);
            }
        }
    }

    function roots(modules) {
        var result = [];
        for (i = 0; i < modules.length; ++i) {
            var module = modules[i]
            if (module.dependencies.lenegth === 0)
                result.push(module);
        }
        return result;
    }

    function traverse(module, libs) {
        if (currentPath.contains(module))
            return;
        currentPath.push(module);

        var moduleLibraryLists = getLibraries(module);
        for (var i = 0; i < moduleLibraryLists.length; ++i) {
            var modLibList = moduleLibraryLists[i];
            for (j = modLibList.length - 1; j >= 0; --j) {
                if (libs.contains(modLibList[j]))
                    modLibList.splice(j, 1);
            }
        }

        var libFilePath = getLibFilePath(module);
        if (libFilePath)
            libs.push(libFilePath);
        for (i = 0; i < moduleLibraryLists.length; ++i)
            libs = libs.concat(moduleLibraryLists[i]);
        libs.sort();

        for (i = 0; i < (revDeps[module] || []).length; ++i)
            traverse(revDeps[module][i], libs);

        m_currentPath.pop();
    }

    setupReverseDependencies(modules);

    // Traverse the debug variants of modules.
    getLibraries = function(module) {
        return [module.dynamicLibrariesDebug, module.staticLibrariesDebug];
    };
    getLibFilePath = function(module) { return module.libFilePathDebug; };
    var rootModules = roots(modules);
    for (var i = 0; i < rootModules.length; ++i)
        traverse(rootModules[i], []);

    // Traverse the release variants of modules.
    getLibraries = function(module) {
        return [module.dynamicLibrariesRelease, module.staticLibrariesRelease];
    };
    getLibFilePath = function(module) { return module.libFilePathRelease; };
    for (i = 0; i < rootModules.length; ++i)
        traverse(rootModules[i], []);
}

function allQt5Modules(qtProps) {
    var nonExistingPrlFiles = [];
    var modules = [];
    var modulesDir = FileInfo.joinPaths(qtProps.mkspecBasePath, "modules");
    var modulePriFiles = File.directoryEntries(modulesDir, File.Files);
    for (var i = 0; i < modulePriFiles.length; ++i) {
        var priFileName = modulePriFiles[i];
        var priFilePath = FileInfo.joinPaths(modulesDir, priFileName);
        var moduleFileNamePrefix = "qt_lib_";
        var pluginFileNamePrefix = "qt_plugin_";
        var moduleFileNameSuffix = ".pri";
        var fileHasPluginPrefix = priFileName.startsWith(pluginFileNamePrefix);
        if (!fileHasPluginPrefix && (!priFileName.startsWith(moduleFileNamePrefix))
                || !priFileName.endsWith(moduleFileNameSuffix)) {
            continue;
        }
        var moduleInfo = makeQtModuleInfo();
        moduleInfo.isPlugin = fileHasPluginPrefix;
        var fileNamePrefix = moduleInfo.isPlugin ? pluginFileNamePrefix : moduleFileNamePrefix;
        moduleInfo.qbsName = priFileName.slice(fileNamePrefix.length, -moduleFileNameSuffix.length);
        if (moduleInfo.isPlugin) {
            moduleInfo.name = moduleInfo.qbsName;
            moduleInfo.isStaticLibrary = true;
        }
        var moduleKeyPrefix = (moduleInfo.isPlugin ? "QT_PLUGIN" : "QT")
                + '.' + moduleInfo.qbsName + '.';
        moduleInfo.qbsName = moduleInfo.qbsName.replace("_private", "-private");
        var hasV2 = false;
        var hasModuleEntry = false;
        var lines = getFileContentsRecursively(priFilePath);
        for (var j = 0; j < lines.length; ++j) {
            var line = lines[j].trim();
            var firstEqualsOffset = line.indexOf('=');
            if (firstEqualsOffset === -1)
                continue;
            var key = line.slice(0, firstEqualsOffset).trim();
            var value = line.slice(firstEqualsOffset + 1).trim();
            if (!key.startsWith(moduleKeyPrefix) || !value)
                continue;
            if (key.endsWith(".name")) {
                moduleInfo.name = value;
            } else if (key.endsWith(".module")) {
                hasModuleEntry = true;
            } else if (key.endsWith(".depends")) {
                moduleInfo.dependencies = splitNonEmpty(value, ' ');
                for (var k = 0; k < moduleInfo.dependencies.length; ++k) {
                    moduleInfo.dependencies[k]
                            = moduleInfo.dependencies[k].replace("_private", "-private");
                }
            } else if (key.endsWith(".module_config")) {
                var elems = splitNonEmpty(value, ' ');
                for (k = 0; k < elems.length; ++k) {
                    var elem = elems[k];
                    if (elem === "no_link")
                        moduleInfo.hasLibrary = false;
                    else if (elem === "staticlib")
                        moduleInfo.isStaticLibrary = true;
                    else if (elem === "internal_module")
                        moduleInfo.isPrivate = true;
                    else if (elem === "v2")
                        hasV2 = true;
                }
            } else if (key.endsWith(".includes")) {
                moduleInfo.includePaths = extractPaths(value, priFilePath);
                for (k = 0; k < moduleInfo.includePaths.length; ++k) {
                    moduleInfo.includePaths[k] = moduleInfo.includePaths[k]
                         .replace("$$QT_MODULE_INCLUDE_BASE", qtProps.includePath)
                         .replace("$$QT_MODULE_LIB_BASE", qtProps.libraryPath);
                }
            } else if (key.endsWith(".DEFINES")) {
                moduleInfo.compilerDefines = splitNonEmpty(value, ' ');
            } else if (key.endsWith(".VERSION")) {
                moduleInfo.version = value;
            } else if (key.endsWith(".plugin_types")) {
                moduleInfo.supportedPluginTypes = splitNonEmpty(value, ' ');
            } else if (key.endsWith(".TYPE")) {
                moduleInfo.pluginData.type = value;
            } else if (key.endsWith(".EXTENDS")) {
                moduleInfo.pluginData["extends"] = splitNonEmpty(value, ' ');
                for (k = 0; k < moduleInfo.pluginData["extends"].length; ++k) {
                    if (moduleInfo.pluginData["extends"][k] === "-") {
                        moduleInfo.pluginData["extends"].splice(k, 1);
                        moduleInfo.pluginData.autoLoad = false;
                        break;
                    }
                }
            } else if (key.endsWith(".CLASS_NAME")) {
                moduleInfo.pluginData.className = value;
            }
        }
        if (hasV2 && !hasModuleEntry)
            moduleInfo.hasLibrary = false;

        // Fix include paths for Apple frameworks.
        // The qt_lib_XXX.pri files contain wrong values for versions < 5.6.
        if (!hasV2 && isFramework(moduleInfo, qtProps)) {
            moduleInfo.includePaths = [];
            var baseIncDir = frameworkHeadersPath(moduleInfo, qtProps);
            if (moduleInfo.isPrivate) {
                baseIncDir = FileInfo.joinPaths(baseIncDir, moduleInfo.version);
                moduleInfo.includePaths.push(baseIncDir,
                                             FileInfo.joinPaths(baseIncDir, moduleInfo.name));
            } else {
                moduleInfo.includePaths.push(baseIncDir);
            }
        }

        setupLibraries(moduleInfo, qtProps, nonExistingPrlFiles);

        modules.push(moduleInfo);
        if (moduleInfo.qbsName === "testlib")
            addTestModule(modules);
        if (moduleInfo.qbsName === "designercomponents-private")
            addDesignerComponentsModule(modules);
    }

    replaceQtLibNamesWithFilePath(modules, qtProps);
    removeDuplicatedDependencyLibs(modules);
    return modules;
}

function extractQbsArchs(module, qtProps) {
    if (qtProps.mkspecName.startsWith("macx-")) {
        var archs = [];
        if (module.libFilePathRelease)
            archs = Utilities.getArchitecturesFromBinary(module.libFilePathRelease);
        return archs;
    }
    var qbsArch = Utilities.canonicalArchitecture(qtProps.architecture);
    if (qbsArch === "arm" && qtProps.mkspecPath.contains("android"))
        qbsArch = "armv7a";

    // Qt4 has "QT_ARCH = windows" in qconfig.pri for both MSVC and mingw.
    if (qbsArch === "windows")
        return []

    return [qbsArch];
}

function qbsTargetPlatformFromQtMkspec(qtProps) {
    var mkspec = qtProps.mkspecName;
    var idx = mkspec.lastIndexOf('/');
    if (idx !== -1)
        mkspec = mkspec.slice(idx + 1);
    if (mkspec.startsWith("aix-"))
        return "aix";
    if (mkspec.startsWith("android-"))
        return "android";
    if (mkspec.startsWith("cygwin-"))
        return "windows";
    if (mkspec.startsWith("darwin-"))
        return "macos";
    if (mkspec.startsWith("freebsd-"))
        return "freebsd";
    if (mkspec.startsWith("haiku-"))
        return "haiku";
    if (mkspec.startsWith(("hpux-")) || mkspec.startsWith(("hpuxi-")))
        return "hpux";
    if (mkspec.startsWith("hurd-"))
        return "hurd";
    if (mkspec.startsWith("integrity-"))
        return "integrity";
    if (mkspec.startsWith("linux-"))
        return "linux";
    if (mkspec.startsWith("macx-")) {
        if (mkspec.startsWith("macx-ios-"))
            return "ios";
        if (mkspec.startsWith("macx-tvos-"))
            return "tvos";
        if (mkspec.startsWith("macx-watchos-"))
            return "watchos";
        return "macos";
    }
    if (mkspec.startsWith("netbsd-"))
        return "netbsd";
    if (mkspec.startsWith("openbsd-"))
        return "openbsd";
    if (mkspec.startsWith("qnx-"))
        return "qnx";
    if (mkspec.startsWith("solaris-"))
        return "solaris";
    if (mkspec.startsWith("vxworks-"))
        return "vxworks";
    if (targetsDesktopWindows(qtProps) || mkspec.startsWith("winrt-"))
        return "windows";
}

function pathToJSLiteral(path) { return JSON.stringify(FileInfo.fromNativeSeparators(path)); }

function defaultQpaPlugin(module, qtProps) {
    if (qtProps.qtMajorVersion < 5)
        return undefined;
    if (qtProps.qtMajorVersion === 5 && qtProps.qtMinorVersion < 8) {
        var qConfigPri = new TextFile(FileInfo.joinPaths(qtProps.mkspecBasePath, "qconfig.pri"));
        var magicString = "QT_DEFAULT_QPA_PLUGIN =";
        while (!qConfigPri.atEof()) {
            var line = qConfigPri.readLine().trim();
            if (line.startsWith(magicString))
                return line.slice(magicString.length).trim();
        }
        qConfigPri.close();
    } else {
        var gtGuiHeadersPath = qtProps.frameworkBuild
                ? FileInfo.joinPaths(qtProps.libraryPath, "QtGui.framework", "Headers")
                : FileInfo.joinPaths(qtProps.includePath, "QtGui");
        var qtGuiConfigHeader = FileInfo.joinPaths(gtGuiHeadersPath, "qtgui-config.h");
        var headerFiles = [];
        headerFiles.push(qtGuiConfigHeader);
        while (headerFiles.length > 0) {
            var filePath = headerFiles.shift();
            var headerFile = new TextFile(filePath, TextFile.ReadOnly);
            var regexp = /^#define QT_QPA_DEFAULT_PLATFORM_NAME "(.+)".*$/;
            var includeRegexp = /^#include "(.+)".*$/;
            while (!headerFile.atEof()) {
                line = headerFile.readLine().trim();
                var match = line.match(regexp);
                if (match)
                    return 'q' + match[1];
                match = line.match(includeRegexp);
                if (match) {
                    var includedFile = match[1];
                    if (!FileInfo.isAbsolutePath(includedFile)) {
                        includedFile = FileInfo.cleanPath(
                                    FileInfo.joinPaths(FileInfo.path(filePath), includedFile));
                    }
                    headerFiles.push(includedFile);
                }
            }
            headerFile.close();
        }
    }

    if (module.isStaticLibrary)
        console.warn("Could not determine default QPA plugin for static Qt.");
}

function libraryFileTag(module, qtProps) {
    if (module.isStaticLibrary)
        return "staticlibrary";
    return isMsvcQt(qtProps) || qtProps.mkspecName.startsWith("win32-g++")
            ? "dynamiclibrary_import" : "dynamiclibrary";
}

function findVariable(content, start) {
    var result = [-1, -1];
    result[0] = content.indexOf('@', start);
    if (result[0] === -1)
        return result;
    result[1] = content.indexOf('@', result[0] + 1);
    if (result[1] === -1) {
        result[0] = -1;
        return result;
    }
    var forbiddenChars = [' ', '\n'];
    for (var i = 0; i < forbiddenChars.length; ++i) {
        var forbidden = forbiddenChars[i];
        var k = content.indexOf(forbidden, result[0] + 1);
        if (k !== -1 && k < result[1])
            return findVariable(content, result[0] + 1);
    }
    return result;
}

function toJSLiteral(v) {
    if (v === undefined)
        return "undefined";
    return JSON.stringify(v);
}

function minVersionJsString(minVersion) {
    return !minVersion ? "original" : toJSLiteral(minVersion);
}

function replaceSpecialValues(content, module, qtProps) {
    var dict = {
        archs: toJSLiteral(extractQbsArchs(module, qtProps)),
        targetPlatform: toJSLiteral(qbsTargetPlatformFromQtMkspec(qtProps)),
        config: toJSLiteral(qtProps.configItems),
        qtConfig: toJSLiteral(qtProps.qtConfigItems),
        binPath: toJSLiteral(qtProps.binaryPath),
        libPath: toJSLiteral(qtProps.libraryPath),
        pluginPath: toJSLiteral(qtProps.pluginPath),
        incPath: toJSLiteral(qtProps.includePath),
        docPath: toJSLiteral(qtProps.documentationPath),
        mkspecName: toJSLiteral(qtProps.mkspecName),
        mkspecPath: toJSLiteral(qtProps.mkspecPath),
        version: toJSLiteral(qtProps.qtVersion),
        libInfix: toJSLiteral(qtProps.qtLibInfix),
        availableBuildVariants: toJSLiteral(qtProps.buildVariant),
        staticBuild: toJSLiteral(qtProps.staticBuild),
        frameworkBuild: toJSLiteral(qtProps.frameworkBuild),
        name: toJSLiteral(moduleNameWithoutPrefix(module)),
        has_library: toJSLiteral(module.hasLibrary),
        dependencies: toJSLiteral(module.dependencies),
        includes: toJSLiteral(module.includePaths),
        staticLibsDebug: toJSLiteral(module.staticLibrariesDebug),
        staticLibsRelease: toJSLiteral(module.staticLibrariesRelease),
        dynamicLibsDebug: toJSLiteral(module.dynamicLibrariesDebug),
        dynamicLibsRelease: toJSLiteral(module.dynamicLibrariesRelease),
        linkerFlagsDebug: toJSLiteral(module.linkerFlagsDebug),
        linkerFlagsRelease: toJSLiteral(module.linkerFlagsRelease),
        libraryPaths: toJSLiteral(module.libraryPaths),
        frameworkPathsDebug: toJSLiteral(module.frameworkPathsDebug),
        frameworkPathsRelease: toJSLiteral(module.frameworkPathsRelease),
        frameworksDebug: toJSLiteral(module.frameworksDebug),
        frameworksRelease: toJSLiteral(module.frameworksRelease),
        libFilePathDebug: toJSLiteral(module.libFilePathDebug),
        libFilePathRelease: toJSLiteral(module.libFilePathRelease),
        libNameForLinkerDebug: toJSLiteral(libNameForLinker(module, qtProps, true)),
        pluginTypes: toJSLiteral(module.supportedPluginTypes),
        moduleConfig: toJSLiteral(module.config),
        libNameForLinkerRelease: toJSLiteral(libNameForLinker(module, qtProps, false)),
        entryPointLibsDebug: toJSLiteral(qtProps.entryPointLibsDebug),
        entryPointLibsRelease: toJSLiteral(qtProps.entryPointLibsRelease),
        minWinVersion: minVersionJsString(qtProps.windowsVersion),
        minMacVersion: minVersionJsString(qtProps.macosVersion),
        minIosVersion: minVersionJsString(qtProps.iosVersion),
        minTvosVersion: minVersionJsString(qtProps.tvosVersion),
        minWatchosVersion: minVersionJsString(qtProps.watchosVersion),
        minAndroidVersion: minVersionJsString(qtProps.androidVersion),
    };

    var additionalContent = "";
    var compilerDefines = toJSLiteral(module.compilerDefines);
    if (module.qbsName === "declarative" || module.qbsName === "quick") {
        var debugMacro = module.qbsName === "declarative" || qtProps.qtMajorVersion < 5
                ? "QT_DECLARATIVE_DEBUG" : "QT_QML_DEBUG";
        var indent = "    ";
        additionalContent = "property bool qmlDebugging: false\n"
                + indent + "property string qmlPath";
        if (qtProps.qmlPath)
            additionalContent += ": " + pathToJSLiteral(qtProps.qmlPath) + '\n';
        else
            additionalContent += '\n';

        additionalContent += indent + "property string qmlImportsPath: "
                + pathToJSLiteral(qtProps.qmlImportPath);

        compilerDefines = "{\n"
                + indent + indent + "var result = " + compilerDefines + ";\n"
                + indent + indent + "if (qmlDebugging)\n"
                + indent + indent + indent + "result.push(\"" + debugMacro + "\");\n"
                + indent + indent + "return result;\n"
                + indent + "}";
    }
    dict.defines = compilerDefines;
    if (module.qbsName === "gui")
        dict.defaultQpaPlugin = toJSLiteral(defaultQpaPlugin(module, qtProps));
    if (module.qbsName === "qml")
        dict.qmlPath = pathToJSLiteral(qtProps.qmlPath);
    if (module.isStaticLibrary && module.qbsName !== "core") {
        if (additionalContent)
            additionalContent += "\n    ";
        additionalContent += "isStaticLibrary: true";
    }
    if (module.isPlugin) {
        dict.className = toJSLiteral(module.pluginData.className);
        dict["extends"] = toJSLiteral(module.pluginData["extends"]);
    }
    if (module.hasLibrary && !isFramework(module, qtProps)) {
        if (additionalContent)
            additionalContent += "\n";
        indent = "    ";
        additionalContent += "Group {\n";
        if (module.isPlugin) {
            additionalContent += indent + indent
                    + "condition: Qt[\"" + module.qbsName + "\"].enableLinking\n";
        }
        additionalContent += indent + indent + "files: [Qt[\"" + module.qbsName + "\"]"
                + ".libFilePath]\n"
                + indent + indent + "filesAreTargets: true\n"
                + indent + indent + "fileTags: [\"" + libraryFileTag(module, qtProps)
                                  + "\"]\n"
                + indent + "}";
    }
    dict.additionalContent = additionalContent;

    for (var pos = findVariable(content, 0); pos[0] !== -1;
         pos = findVariable(content, pos[0])) {
        var replacement = dict[content.slice(pos[0] + 1, pos[1])] || "";
        content = content.slice(0, pos[0]) + replacement + content.slice(pos[1] + 1);
        pos[0] += replacement.length;
    }
    return content;
}

function copyTemplateFile(fileName, targetDirectory, qtProps, location, allFiles, module, pluginMap,
                          nonEssentialPlugins)
{
    if (!File.makePath(targetDirectory)) {
        throw "Cannot create directory '" + toNative(targetDirectory) + "'.";
    }
    var sourceFile = new TextFile(FileInfo.joinPaths(location, "templates", fileName),
                                  TextFile.ReadOnly);
    var newContent = sourceFile.readAll();
    if (module) {
        newContent = replaceSpecialValues(newContent, module, qtProps);
    } else {
        newContent = newContent.replace("@allPluginsByType@",
                                        '(' + toJSLiteral(pluginMap) + ')');
        newContent = newContent.replace("@nonEssentialPlugins@",
                                        toJSLiteral(nonEssentialPlugins));
    }
    sourceFile.close();
    var targetPath = FileInfo.joinPaths(targetDirectory, fileName);
    allFiles.push(targetPath);
    var targetFile = new TextFile(targetPath, TextFile.WriteOnly);
    targetFile.write(newContent);
    targetFile.close();
}

function setupOneQt(qmakeFilePath, outputBaseDir, uniquify, location, qbs) {
    if (!File.exists(qmakeFilePath))
        throw "The specified qmake file path '" + toNative(qmakeFilePath) + "' does not exist.";
    var qtProps = getQtProperties(qmakeFilePath, qbs);
    var modules = qtProps.qtMajorVersion < 5 ? allQt4Modules(qtProps) : allQt5Modules(qtProps);
    var pluginsByType = {};
    var nonEssentialPlugins = [];
    for (var i = 0; i < modules.length; ++i) {
        var m = modules[i];
        if (m.isPlugin) {
            if (!pluginsByType[m.pluginData.type])
                pluginsByType[m.pluginData.type] = [];
            pluginsByType[m.pluginData.type].push(m.name);
            if (!m.pluginData.autoLoad)
                nonEssentialPlugins.push(m.name);
        }
    }

    var relativeSearchPath = uniquify ? Utilities.getHash(qmakeFilePath) : "";
    var qbsQtModuleBaseDir = FileInfo.joinPaths(outputBaseDir, relativeSearchPath, "modules", "Qt");
    if (File.exists(qbsQtModuleBaseDir))
        File.remove(qbsQtModuleBaseDir);

    var allFiles = [];
    copyTemplateFile("QtModule.qbs", qbsQtModuleBaseDir, qtProps, location, allFiles);
    copyTemplateFile("QtPlugin.qbs", qbsQtModuleBaseDir, qtProps, location, allFiles);
    copyTemplateFile("plugin_support.qbs", FileInfo.joinPaths(qbsQtModuleBaseDir, "plugin_support"),
                     qtProps, location, allFiles, undefined, pluginsByType, nonEssentialPlugins);

    for (i = 0; i < modules.length; ++i) {
        var module = modules[i];
        var qbsQtModuleDir = FileInfo.joinPaths(qbsQtModuleBaseDir, module.qbsName);
        var moduleTemplateFileName;
        if (module.qbsName === "core") {
            moduleTemplateFileName = "core.qbs";
            copyTemplateFile("moc.js", qbsQtModuleDir, qtProps, location, allFiles);
            copyTemplateFile("qdoc.js", qbsQtModuleDir, qtProps, location, allFiles);
        } else if (module.qbsName === "gui") {
            moduleTemplateFileName = "gui.qbs";
        } else if (module.qbsName === "scxml") {
            moduleTemplateFileName = "scxml.qbs";
        } else if (module.qbsName === "dbus") {
            moduleTemplateFileName = "dbus.qbs";
            copyTemplateFile("dbus.js", qbsQtModuleDir, qtProps, location, allFiles);
        } else if (module.qbsName === "qml") {
            moduleTemplateFileName = "qml.qbs";
            copyTemplateFile("qml.js", qbsQtModuleDir, qtProps, location, allFiles);
            var qmlcacheStr = "qmlcache";
            if (File.exists(FileInfo.joinPaths(qtProps.binaryPath,
                                               "qmlcachegen" + exeSuffix(qbs)))) {
                copyTemplateFile(qmlcacheStr + ".qbs",
                                 FileInfo.joinPaths(qbsQtModuleBaseDir, qmlcacheStr), qtProps,
                                 location, allFiles);
            }
        } else if (module.qbsName === "quick") {
            moduleTemplateFileName = "quick.qbs";
            copyTemplateFile("quick.js", qbsQtModuleDir, qtProps, location, allFiles);
        } else if (module.isPlugin) {
            moduleTemplateFileName = "plugin.qbs";
        } else {
            moduleTemplateFileName = "module.qbs";
        }
        copyTemplateFile(moduleTemplateFileName, qbsQtModuleDir, qtProps, location, allFiles,
                         module);
    }

    // Note that it's not strictly necessary to copy this one, as it has no variable content.
    // But we do it anyway for consistency.
    copyTemplateFile("android_support.qbs",
                     FileInfo.joinPaths(qbsQtModuleBaseDir, "android_support"),
                     qtProps, location, allFiles);
    return relativeSearchPath;
}

function doSetup(qmakeFilePaths, outputBaseDir, location, qbs) {
    qmakeFilePaths = getQmakeFilePaths(qmakeFilePaths, qbs);
    if (!qmakeFilePaths || qmakeFilePaths.length === 0)
        return [];
    var uniquifySearchPath = qmakeFilePaths.length > 1;
    var searchPaths = [];
    for (var i = 0; i < qmakeFilePaths.length; ++i) {
        try {
            console.info("Setting up Qt at '" + toNative(qmakeFilePaths[i]) + "'...");
            var searchPath = setupOneQt(qmakeFilePaths[i], outputBaseDir, uniquifySearchPath,
                                        location, qbs);
            if (searchPath !== undefined) {
                searchPaths.push(searchPath);
                console.info("Qt was set up successfully.");
            }
        } catch (e) {
            console.warn("Error setting up Qt for '" + toNative(qmakeFilePaths[i]) + "': " + e);
        }
    }
    return searchPaths;
}
