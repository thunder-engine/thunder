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

import qbs.DarwinTools
import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.PathTools
import qbs.Probes
import qbs.PropertyList
import qbs.TextFile
import qbs.Utilities
import "darwin.js" as Darwin
import "gcc.js" as Gcc

UnixGCC {
    condition: false

    Depends { name: "xcode"; required: qbs.toolchain && qbs.toolchain.contains("xcode") }

    Probes.BinaryProbe {
        id: lipoProbe
        condition: !_skipAllChecks
        names: [lipoName]
        platformSearchPaths: {
            var paths = (xcode.present && xcode.devicePlatformPath)
                    ? [xcode.devicePlatformPath + "/Developer/usr/bin"]
                    : [];
            return paths.concat([toolchainInstallPath, "/usr/bin"]);
        }
    }

    property string lipoPathPrefix: Gcc.pathPrefix(lipoProbe.found
                                                   ? lipoProbe.path
                                                   : toolchainInstallPath, toolchainPrefix)

    lipoName: "lipo"
    lipoPath: lipoPathPrefix + lipoName
    property bool enableAggregationRules: product.aggregate && !product.multiplexConfigurationId

    targetVendor: "apple"
    targetSystem: "darwin"
    targetAbi: "macho"
    imageFormat: "macho"

    cxxStandardLibrary: libcxxAvailable ? "libc++" : base
    loadableModulePrefix: ""
    loadableModuleSuffix: ".bundle"
    dynamicLibrarySuffix: ".dylib"

    Properties {
        condition: product.multiplexByQbsProperties.contains("buildVariants")
                   && qbs.buildVariants && qbs.buildVariants.length > 1
                   && (!product.aggregate || !!product.multiplexConfigurationId)
                   && qbs.buildVariant !== "release"
        variantSuffix: "_" + qbs.buildVariant
    }

    separateDebugInformation: true
    debugInfoBundleSuffix: ".dSYM"
    debugInfoSuffix: ".dwarf"
    rpathOrigin: "@loader_path"
    useRPathLink: !minimumDarwinVersion
                  || Utilities.versionCompare(minimumDarwinVersion, "10.5") < 0
    rpathLinkFlag: "-L"

    toolchainInstallPath: xcode.present
                          ? FileInfo.joinPaths(xcode.toolchainPath, "usr", "bin") : base
    sysroot: xcode.present ? xcode.sdkPath : base
    sysrootFlags: sysroot ? ["-isysroot", sysroot] : []

    setupBuildEnvironment: {
        for (var key in product.cpp.buildEnv) {
            v = new ModUtils.EnvironmentVariable(key);
            v.value = product.cpp.buildEnv[key];
            v.set();
        }
    }

    property var defaultInfoPlist: {
        var dict = {};

        if (qbs.targetOS.contains("macos")) {
            dict["NSPrincipalClass"] = "NSApplication"; // needed for Retina display support

            if (minimumMacosVersion)
                dict["LSMinimumSystemVersion"] = minimumMacosVersion;
        }

        if (qbs.targetOS.containsAny(["ios", "tvos"])) {
            dict["LSRequiresIPhoneOS"] = true;

            if (xcode.platformType === "device") {
                if (qbs.targetOS.contains("ios"))
                    dict["UIRequiredDeviceCapabilities"] = ["armv7"];

                if (qbs.targetOS.contains("tvos"))
                    dict["UIRequiredDeviceCapabilities"] = ["arm64"];
            }
        }

        if (xcode.present) {
            var targetDevices = DarwinTools.targetedDeviceFamily(xcode.targetDevices);
            if (qbs.targetOS.contains("ios"))
                dict["UIDeviceFamily"] = targetDevices;

            if (qbs.targetOS.containsAny(["ios", "watchos"])) {
                var orientations = [
                    "UIInterfaceOrientationPortrait",
                    "UIInterfaceOrientationPortraitUpsideDown",
                    "UIInterfaceOrientationLandscapeLeft",
                    "UIInterfaceOrientationLandscapeRight"
                ];

                if (targetDevices.contains("ipad"))
                    dict["UISupportedInterfaceOrientations~ipad"] = orientations;

                if (targetDevices.contains("watch"))
                    dict["UISupportedInterfaceOrientations"] = orientations.slice(0, 2);

                if (targetDevices.contains("iphone")) {
                    orientations.splice(1, 1);
                    dict["UISupportedInterfaceOrientations"] = orientations;
                }
            }
        }

        return dict;
    }

    targetLinkerFlags: darwinArchFlags.concat(minimumDarwinVersionLinkerFlags)

    targetAssemblerFlags: !assemblerHasTargetOption ? darwinArchFlags : base;

    targetDriverFlags: !compilerHasTargetOption ? legacyTargetDriverFlags : base

    property stringList legacyTargetDriverFlags:
        base.concat(darwinArchFlags).concat(minimumDarwinVersionCompilerFlags)

    // private properties
    readonly property stringList darwinArchFlags: targetArch ? ["-arch", targetArch] : []

    readonly property stringList minimumDarwinVersionCompilerFlags:
        (minimumDarwinVersionCompilerFlag && minimumDarwinVersion)
        ? [minimumDarwinVersionCompilerFlag + "=" + minimumDarwinVersion] : []

    readonly property stringList minimumDarwinVersionLinkerFlags:
        (minimumDarwinVersionLinkerFlag && minimumDarwinVersion)
        ? [minimumDarwinVersionLinkerFlag, minimumDarwinVersion] : []

    readonly property var buildEnv: {
        var env = {
            "ARCHS_STANDARD": xcode.standardArchitectures,
            "EXECUTABLE_NAME": product.targetName,
            "LANG": "en_US.US-ASCII",
            "PRODUCT_NAME": product.name
        }

        // Set the corresponding environment variable even if the minimum OS version is undefined,
        // because this indicates the default deployment target for that OS
        if (qbs.targetOS.contains("ios") && minimumIosVersion)
            env["IPHONEOS_DEPLOYMENT_TARGET"] = minimumIosVersion;
        if (qbs.targetOS.contains("macos") && minimumMacosVersion)
            env["MACOSX_DEPLOYMENT_TARGET"] = minimumMacosVersion;
        if (qbs.targetOS.contains("watchos") && minimumWatchosVersion)
            env["WATCHOS_DEPLOYMENT_TARGET"] = minimumWatchosVersion;
        if (qbs.targetOS.contains("tvos") && minimumTvosVersion)
            env["TVOS_DEPLOYMENT_TARGET"] = minimumTvosVersion;

        if (xcode.present)
            env["TARGETED_DEVICE_FAMILY"] = DarwinTools.targetedDeviceFamily(xcode.targetDevices);
        return env;
    }

    property string minimumDarwinVersion
    property string minimumDarwinVersionCompilerFlag
    property string minimumDarwinVersionLinkerFlag

    property bool libcxxAvailable: qbs.toolchain.contains("clang") && cxxLanguageVersion !== "c++98"

    Rule {
        condition: enableAggregationRules
        inputsFromDependencies: ["application"]
        multiplex: true

        outputFileTags: ["bundle.input", "application", "primary", "debuginfo_app",
                         "debuginfo_bundle", "bundle.variant_symlink", "debuginfo_plist"]
        outputArtifacts: Darwin.lipoOutputArtifacts(product, inputs, "application", "app")

        prepare: Darwin.prepareLipo.apply(Darwin, arguments)
    }

    Rule {
        condition: enableAggregationRules
        inputsFromDependencies: ["loadablemodule"]
        multiplex: true

        outputFileTags: ["bundle.input", "loadablemodule", "primary", "debuginfo_loadablemodule"]
        outputArtifacts: Darwin.lipoOutputArtifacts(product, inputs, "loadablemodule",
                                                                     "loadablemodule")

        prepare: Darwin.prepareLipo.apply(Darwin, arguments)
    }

    Rule {
        condition: enableAggregationRules
        inputsFromDependencies: ["dynamiclibrary"]
        multiplex: true

        outputFileTags: ["bundle.input", "dynamiclibrary", "dynamiclibrary_symbols", "primary",
                         "debuginfo_dll","debuginfo_bundle","bundle.variant_symlink",
                         "debuginfo_plist"]
        outputArtifacts: Darwin.lipoOutputArtifacts(product, inputs, "dynamiclibrary", "dll")

        prepare: Darwin.prepareLipo.apply(Darwin, arguments)
    }

    Rule {
        condition: enableAggregationRules
        inputsFromDependencies: ["staticlibrary"]
        multiplex: true

        outputFileTags: ["bundle.input", "staticlibrary", "primary"]
        outputArtifacts: Darwin.lipoOutputArtifacts(product, inputs, "staticlibrary")

        prepare: Darwin.prepareLipo.apply(Darwin, arguments)
    }

    Rule {
        condition: qbs.targetOS.contains("darwin")
        multiplex: true

        Artifact {
            filePath: product.name + "-cpp-Info.plist"
            fileTags: ["partial_infoplist"]
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.silent = true;
            cmd.inputData = product.cpp.defaultInfoPlist;
            cmd.outputFilePath = output.filePath;
            cmd.sourceCode = function() {
                var plist = new PropertyList();
                try {
                    plist.readFromObject(inputData);
                    plist.writeToFile(outputFilePath, "xml1");
                } finally {
                    plist.clear();
                }
            };
            return [cmd];
        }
    }
}
