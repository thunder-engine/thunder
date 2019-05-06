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

// base for Cpp modules
import qbs.ModUtils
import qbs.Utilities
import qbs.WindowsUtils

import "setuprunenv.js" as SetupRunEnv

Module {
    condition: false

    Depends { name: "cpufeatures" }

    version: compilerVersion
    property string compilerVersion:
        [compilerVersionMajor, compilerVersionMinor, compilerVersionPatch].join(".")
    property int compilerVersionMajor
    property int compilerVersionMinor
    property int compilerVersionPatch
    property string warningLevel : 'all' // 'none', 'all'
    property bool treatWarningsAsErrors : false
    property bool enableSuspiciousLinkerFlagWarnings: true
    property string architecture: qbs.architecture
    property string endianness
    property string machineType // undocumented
    property string imageFormat // undocumented
    property string optimization: qbs.optimization
    property bool debugInformation: qbs.debugInformation
    property bool enableReproducibleBuilds: false
    property bool separateDebugInformation: false
    property pathList prefixHeaders
    property bool useCPrecompiledHeader: true
    property bool useCxxPrecompiledHeader: true
    property bool useObjcPrecompiledHeader: true
    property bool useObjcxxPrecompiledHeader: true

    property bool treatSystemHeadersAsDependencies: false

    property stringList defines
    property stringList platformDefines: qbs.enableDebugCode ? [] : ["NDEBUG"]
    property stringList compilerDefines: compilerDefinesByLanguage
                                  ? ModUtils.flattenDictionary(compilerDefinesByLanguage["c"])
                                  : []
    property var compilerDefinesByLanguage: undefined
    PropertyOptions {
        name: "compilerDefinesByLanguage"
        description: "preprocessor macros that are defined when using this particular compiler"
    }
    property stringList enableCompilerDefinesByLanguage: []

    property string windowsApiCharacterSet
    property string windowsApiFamily
    property stringList windowsApiAdditionalPartitions
    property bool requireAppContainer

    property string minimumWindowsVersion
    PropertyOptions {
        name: "minimumWindowsVersion"
        description: "A version number in the format [major].[minor] indicating the earliest \
                        version of Windows that the product should run on. Defines WINVER, \
                        _WIN32_WINNT, and _WIN32_WINDOWS, and applies a version number to the \
                        linker flags /SUBSYSTEM and /OSVERSION for MSVC or \
                        --major-subsystem-version, --minor-subsystem-version, \
                        --major-os-version and --minor-os-version for MinGW. \
                        If undefined, compiler defaults will be used."
    }

    property string minimumOsxVersion

    property string minimumMacosVersion: minimumOsxVersion
    PropertyOptions {
        name: "minimumMacosVersion"
        description: "a version number in the format [major].[minor] indicating the earliest \
                        version of macOS that the product should run on. passes -mmacosx-version-min=<version> \
                        to the compiler. if undefined, compiler defaults will be used."
    }

    property string minimumIosVersion
    PropertyOptions {
        name: "minimumIosVersion"
        description: "a version number in the format [major].[minor] indicating the earliest \
                        version of iOS that the product should run on. passes -miphoneos-version-min=<version> \
                        to the compiler. if undefined, compiler defaults will be used."
    }

    property string minimumWatchosVersion
    PropertyOptions {
        name: "minimumWatchosVersion"
        description: "a version number in the format [major].[minor] indicating the earliest \
                        version of watchOS that the product should run on. if undefined, compiler \
                        defaults will be used."
    }

    property string minimumTvosVersion
    PropertyOptions {
        name: "minimumTvosVersion"
        description: "a version number in the format [major].[minor] indicating the earliest \
                        version of tvOS that the product should run on. if undefined, compiler \
                        defaults will be used."
    }

    property string minimumAndroidVersion
    PropertyOptions {
        name: "minimumAndroidVersion"
        description: "a version number in the format [major].[minor] indicating the earliest \
                        version of Android that the product should run on. this value is converted into an SDK \
                        version which is then written to AndroidManifest.xml."
    }

    property string maximumAndroidVersion
    PropertyOptions {
        name: "maximumAndroidVersion"
        description: "a version number in the format [major].[minor] indicating the latest \
                        version of Android that the product should run on. this value is converted into an SDK \
                        version which is then written to AndroidManifest.xml. if undefined, no upper limit will \
                        be set."
    }

    property pathList includePaths
    property pathList systemIncludePaths
    property pathList distributionIncludePaths
    property pathList compilerIncludePaths
    property pathList libraryPaths
    property pathList distributionLibraryPaths
    property pathList compilerLibraryPaths
    property pathList frameworkPaths
    property pathList systemFrameworkPaths
    property pathList distributionFrameworkPaths
    property pathList compilerFrameworkPaths
    property stringList systemRunPaths: []

    property string assemblerName
    property string assemblerPath: assemblerName
    property string compilerName
    property string compilerPath: compilerName
    property var compilerPathByLanguage
    property stringList compilerWrapper
    property string linkerName
    property string linkerPath: linkerName
    property stringList linkerWrapper
    property string staticLibraryPrefix: ""
    property string dynamicLibraryPrefix: ""
    property string loadableModulePrefix: ""
    property string executablePrefix: ""
    property string staticLibrarySuffix: ""
    property string dynamicLibrarySuffix: ""
    property string loadableModuleSuffix: ""
    property string executableSuffix: ""
    property string debugInfoSuffix: ""
    property string debugInfoBundleSuffix: ""
    property string variantSuffix: ""
    property string dynamicLibraryImportSuffix: ".lib"
    property bool createSymlinks: true
    property stringList dynamicLibraries // list of names, will be linked with -lname
    property stringList staticLibraries // list of static library files
    property stringList frameworks // list of frameworks, will be linked with '-framework <name>'
    property stringList weakFrameworks // list of weakly-linked frameworks, will be linked with '-weak_framework <name>'
    property string rpathOrigin
    property stringList rpaths
    property string sonamePrefix: ""
    property bool useRPaths: true
    property bool useRPathLink
    property string rpathLinkFlag
    property bool discardUnusedData

    property stringList assemblerFlags
    PropertyOptions {
        name: "assemblerFlags"
        description: "additional flags for the assembler"
    }

    property stringList cppFlags
    PropertyOptions {
        name: "cppFlags"
        description: "additional flags for the C preprocessor"
    }

    property stringList cFlags
    PropertyOptions {
        name: "cFlags"
        description: "additional flags for the C compiler"
    }

    property stringList cxxFlags
    PropertyOptions {
        name: "cxxFlags"
        description: "additional flags for the C++ compiler"
    }

    property stringList objcFlags
    PropertyOptions {
        name: "objcFlags"
        description: "additional flags for the Objective-C compiler"
    }

    property stringList objcxxFlags
    PropertyOptions {
        name: "objcxxFlags"
        description: "additional flags for the Objective-C++ compiler"
    }
    property stringList commonCompilerFlags
    PropertyOptions {
        name: "commonCompilerFlags"
        description: "flags added to all compilation independently of the language"
    }

    property stringList linkerFlags
    PropertyOptions {
        name: "linkerFlags"
        description: "additional linker flags"
    }

    property stringList driverFlags
    PropertyOptions {
        name: "driverFlags"
        description: "additional compiler driver flags"
    }

    property stringList driverLinkerFlags
    PropertyOptions {
        name: "driverLinkerFlags"
        description: "additional compiler driver flags used for linking only"
    }

    property bool positionIndependentCode: true
    PropertyOptions {
        name: "positionIndependentCode"
        description: "generate position independent code"
    }

    property string entryPoint
    PropertyOptions {
        name: "entryPoint"
        description: "entry point symbol for an executable or dynamic library"
    }

    property string runtimeLibrary
    PropertyOptions {
        name: "runtimeLibrary"
        description: "determine which runtime library to use"
        allowedValues: ['static', 'dynamic']
    }

    property string visibility: 'default'
    PropertyOptions {
        name: "visibility"
        description: "export symbols visibility level"
        allowedValues: ['default', 'hidden', 'hiddenInlines', 'minimal']
    }

    property stringList cLanguageVersion
    PropertyOptions {
        name: "cLanguageVersion"
        allowedValues: ["c89", "c99", "c11"]
        description: "The version of the C standard with which the code must comply."
    }

    property stringList cxxLanguageVersion
    PropertyOptions {
        name: "cxxLanguageVersion"
        allowedValues: ["c++98", "c++11", "c++14", "c++17"]
        description: "The version of the C++ standard with which the code must comply."
    }

    property bool useLanguageVersionFallback
    PropertyOptions {
        name: "useLanguageVersionFallback"
        description: "whether to explicitly use the language standard version fallback values in " +
                     "compiler command line invocations"
    }

    property string cxxStandardLibrary
    PropertyOptions {
        name: "cxxStandardLibrary"
        allowedValues: ["libstdc++", "libc++"]
        description: "version of the C++ standard library to use"
    }

    property bool enableExceptions: true
    PropertyOptions {
        name: "enableExceptions"
        description: "enable/disable exception handling (enabled by default)"
    }

    property string exceptionHandlingModel: "default"
    PropertyOptions {
        name: "exceptionHandlingModel"
        description: "the kind of exception handling to be used by the compiler"
    }

    property bool enableRtti

    // Platform properties. Those are intended to be set by the toolchain setup
    // and are prepended to the corresponding user properties.
    property stringList platformAssemblerFlags
    property stringList platformCommonCompilerFlags
    property stringList platformCFlags
    property stringList platformCxxFlags
    property stringList platformObjcFlags
    property stringList platformObjcxxFlags
    property stringList platformLinkerFlags
    property stringList platformDriverFlags

    // Apple platforms properties
    property bool automaticReferenceCounting
    PropertyOptions {
        name: "automaticReferenceCounting"
        description: "whether to enable Automatic Reference Counting (ARC) for Objective-C"
    }

    property bool requireAppExtensionSafeApi
    PropertyOptions {
        name: "requireAppExtensionSafeApi"
        description: "whether to require app-extension-safe APIs only"
    }

    property bool allowUnresolvedSymbols

    property bool combineCSources: false
    property bool combineCxxSources: false
    property bool combineObjcSources: false
    property bool combineObjcxxSources: false

    property stringList targetAssemblerFlags
    property stringList targetDriverFlags
    property stringList targetLinkerFlags

    property bool _skipAllChecks: false // Internal

    property bool validateTargetTriple: true

    // TODO: The following four rules could use a convenience base item if rule properties
    //       were available in Artifact items and prepare scripts.
    Rule {
        multiplex: true
        inputs: ["c.combine"]
        Artifact {
            filePath: "amalgamated_" + product.targetName + ".c"
            fileTags: ["c"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "creating " + output.fileName;
            cmd.highlight = "codegen";
            cmd.sourceCode = function() {
                ModUtils.mergeCFiles(inputs["c.combine"], output.filePath);
            };
            return [cmd];
        }
    }
    Rule {
        multiplex: true
        inputs: ["cpp.combine"]
        Artifact {
            filePath: "amalgamated_" + product.targetName + ".cpp"
            fileTags: ["cpp"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "creating " + output.fileName;
            cmd.highlight = "codegen";
            cmd.sourceCode = function() {
                ModUtils.mergeCFiles(inputs["cpp.combine"], output.filePath);
            };
            return [cmd];
        }
    }
    Rule {
        multiplex: true
        inputs: ["objc.combine"]
        Artifact {
            filePath: "amalgamated_" + product.targetName + ".m"
            fileTags: ["objc"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "creating " + output.fileName;
            cmd.highlight = "codegen";
            cmd.sourceCode = function() {
                ModUtils.mergeCFiles(inputs["objc.combine"], output.filePath);
            };
            return [cmd];
        }
    }
    Rule {
        multiplex: true
        inputs: ["objcpp.combine"]
        Artifact {
            filePath: "amalgamated_" + product.targetName + ".mm"
            fileTags: ["objccpp"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "creating " + output.fileName;
            cmd.highlight = "codegen";
            cmd.sourceCode = function() {
                ModUtils.mergeCFiles(inputs["objcpp.combine"], output.filePath);
            };
            return [cmd];
        }
    }

    FileTagger {
        patterns: ["*.c"]
        fileTags: combineCSources ? ["c.combine"] : ["c"]
    }

    FileTagger {
        patterns: ["*.C", "*.cpp", "*.cxx", "*.c++", "*.cc"]
        fileTags: combineCxxSources ? ["cpp.combine"] : ["cpp"]
    }

    FileTagger {
        patterns: ["*.m"]
        fileTags: combineObjcSources ? ["objc.combine"] : ["objc"]
    }

    FileTagger {
        patterns: ["*.mm"]
        fileTags: combineObjcxxSources ? ["objcpp.combine"] : ["objcpp"]
    }

    FileTagger {
        patterns: ["*.h", "*.H", "*.hpp", "*.hxx", "*.h++"]
        fileTags: ["hpp"]
    }

    validate: {
        var validator = new ModUtils.PropertyValidator("cpp");
        validator.setRequiredProperty("architecture", architecture,
                                      "you might want to re-run 'qbs-setup-toolchains'");
        validator.addCustomValidator("architecture", architecture, function (value) {
            return !architecture || architecture === Utilities.canonicalArchitecture(architecture);
        }, "'" + architecture + "' is invalid. You must use the canonical name '" +
        Utilities.canonicalArchitecture(architecture) + "'");
        validator.setRequiredProperty("endianness", endianness);
        validator.setRequiredProperty("compilerDefinesByLanguage", compilerDefinesByLanguage);
        validator.setRequiredProperty("compilerVersion", compilerVersion);
        validator.setRequiredProperty("compilerVersionMajor", compilerVersionMajor);
        validator.setRequiredProperty("compilerVersionMinor", compilerVersionMinor);
        validator.setRequiredProperty("compilerVersionPatch", compilerVersionPatch);
        validator.addVersionValidator("compilerVersion", compilerVersion, 3, 3);
        validator.addRangeValidator("compilerVersionMajor", compilerVersionMajor, 1);
        validator.addRangeValidator("compilerVersionMinor", compilerVersionMinor, 0);
        validator.addRangeValidator("compilerVersionPatch", compilerVersionPatch, 0);
        if (minimumWindowsVersion) {
            validator.addVersionValidator("minimumWindowsVersion", minimumWindowsVersion, 2, 2);
            validator.addCustomValidator("minimumWindowsVersion", minimumWindowsVersion, function (v) {
                return !v || v === WindowsUtils.canonicalizeVersion(v);
            }, "'" + minimumWindowsVersion + "' is invalid. Did you mean '" +
            WindowsUtils.canonicalizeVersion(minimumWindowsVersion) + "'?");
        }
        validator.validate();

        if (minimumWindowsVersion && !WindowsUtils.isValidWindowsVersion(minimumWindowsVersion)) {
            console.warn("Unknown Windows version '" + minimumWindowsVersion
                + "'; expected one of: "
                + WindowsUtils.knownWindowsVersions().map(function (a) {
                    return '"' + a + '"'; }).join(", ")
                + ". See https://docs.microsoft.com/en-us/windows/desktop/SysInfo/operating-system-version");
        }
    }

    setupRunEnvironment: {
        SetupRunEnv.setupRunEnvironment(product, config);
    }

    Parameter {
        property bool link
    }
    Parameter {
        property bool linkWholeArchive
    }
    Parameter {
        property string symbolLinkMode
    }
}
