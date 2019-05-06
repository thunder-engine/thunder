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

import qbs.BundleTools
import qbs.DarwinTools
import qbs.Environment
import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.PropertyList
import qbs.TextFile
import qbs.Utilities
import "bundle.js" as Bundle

Module {
    Depends { name: "xcode"; required: false; }

    Probe {
        id: bundleSettingsProbe
        condition: qbs.targetOS.contains("darwin")

        property string xcodeDeveloperPath: xcode.developerPath
        property var xcodeArchSettings: xcode._architectureSettings
        property string productTypeIdentifier: _productTypeIdentifier
        property bool useXcodeBuildSpecs: _useXcodeBuildSpecs
        property bool isMacOs: qbs.targetOS.contains("macos")
        property bool xcodePresent: xcode.present

        // Note that we include several settings pointing to properties which reference the output
        // of this probe (WRAPPER_NAME, WRAPPER_EXTENSION, etc.). This is to ensure that derived
        // properties take into account the value of these settings if the user customized them.
        property var additionalSettings: ({
            "DEVELOPMENT_LANGUAGE": "English",
            "EXECUTABLE_VARIANT_SUFFIX": "", // e.g. _debug, _profile
            "FRAMEWORK_VERSION": frameworkVersion,
            "GENERATE_PKGINFO_FILE": generatePackageInfo !== undefined
                                     ? (generatePackageInfo ? "YES" : "NO")
                                     : undefined,
            "PRODUCT_NAME": product.targetName,
            "LOCAL_APPS_DIR": Environment.getEnv("HOME") + "/Applications",
            "LOCAL_LIBRARY_DIR": Environment.getEnv("HOME") + "/Library",
            "TARGET_BUILD_DIR": product.buildDirectory,
            "WRAPPER_NAME": bundleName,
            "WRAPPER_EXTENSION": extension
        })

        // Outputs
        property var xcodeSettings: ({})
        property var productTypeIdentifierChain: []

        configure: {
            var specsPath = path;
            var specsSeparator = "-";
            if (xcodeDeveloperPath && useXcodeBuildSpecs) {
                specsPath = xcodeDeveloperPath
                        + "/Platforms/MacOSX.platform/Developer/Library/Xcode/Specifications";
                specsSeparator = " ";
            }

            var reader = new Bundle.XcodeBuildSpecsReader(specsPath,
                                                          specsSeparator,
                                                          additionalSettings,
                                                          !isMacOs);
            var settings = reader.expandedSettings(productTypeIdentifier,
                                                   xcodePresent
                                                   ? xcodeArchSettings
                                                   : {});
            var chain = reader.productTypeIdentifierChain(productTypeIdentifier);
            if (settings && chain) {
                xcodeSettings = settings;
                productTypeIdentifierChain = chain;
                found = true;
            } else {
                xcodeSettings = {};
                productTypeIdentifierChain = [];
                found = false;
            }
        }
    }

    additionalProductTypes: !(product.multiplexed || product.aggregate)
                            || !product.multiplexConfigurationId ? ["bundle.content"] : []

    property bool isBundle: !product.consoleApplication && qbs.targetOS.contains("darwin")

    readonly property bool isShallow: bundleSettingsProbe.xcodeSettings["SHALLOW_BUNDLE"] === "YES"

    property string identifierPrefix: "org.example"
    property string identifier: [identifierPrefix, Utilities.rfc1034Identifier(product.targetName)].join(".")

    property string extension: bundleSettingsProbe.xcodeSettings["WRAPPER_EXTENSION"]

    property string packageType: Bundle.packageType(_productTypeIdentifier)

    property string signature: "????" // legacy creator code in Mac OS Classic (CFBundleSignature), can be ignored

    property string bundleName: bundleSettingsProbe.xcodeSettings["WRAPPER_NAME"]

    property string frameworkVersion: {
        var n = parseInt(product.version, 10);
        return isNaN(n) ? bundleSettingsProbe.xcodeSettings["FRAMEWORK_VERSION"] : String(n);
    }

    property bool generatePackageInfo: {
        // Make sure to return undefined as default to indicate "not set"
        var genPkgInfo = bundleSettingsProbe.xcodeSettings["GENERATE_PKGINFO_FILE"];
        if (genPkgInfo)
            return genPkgInfo === "YES";
    }

    property pathList publicHeaders
    property pathList privateHeaders
    property pathList resources

    property var infoPlist
    property bool processInfoPlist: true
    property bool embedInfoPlist: product.consoleApplication && !isBundle
    property string infoPlistFormat: qbs.targetOS.contains("macos") ? "same-as-input" : "binary1"

    property string localizedResourcesFolderSuffix: ".lproj"

    property string lsregisterName: "lsregister"
    property string lsregisterPath: FileInfo.joinPaths(
                                        "/System/Library/Frameworks/CoreServices.framework" +
                                        "/Versions/A/Frameworks/LaunchServices.framework" +
                                        "/Versions/A/Support", lsregisterName);

    // all paths are relative to the directory containing the bundle
    readonly property string infoPlistPath: bundleSettingsProbe.xcodeSettings["INFOPLIST_PATH"]
    readonly property string infoStringsPath: bundleSettingsProbe.xcodeSettings["INFOSTRINGS_PATH"]
    readonly property string pbdevelopmentPlistPath: bundleSettingsProbe.xcodeSettings["PBDEVELOPMENTPLIST_PATH"]
    readonly property string pkgInfoPath: bundleSettingsProbe.xcodeSettings["PKGINFO_PATH"]
    readonly property string versionPlistPath: bundleSettingsProbe.xcodeSettings["VERSIONPLIST_PATH"]

    readonly property string executablePath: bundleSettingsProbe.xcodeSettings["EXECUTABLE_PATH"]

    readonly property string contentsFolderPath: bundleSettingsProbe.xcodeSettings["CONTENTS_FOLDER_PATH"]
    readonly property string documentationFolderPath: bundleSettingsProbe.xcodeSettings["DOCUMENTATION_FOLDER_PATH"]
    readonly property string executableFolderPath: bundleSettingsProbe.xcodeSettings["EXECUTABLE_FOLDER_PATH"]
    readonly property string executablesFolderPath: bundleSettingsProbe.xcodeSettings["EXECUTABLES_FOLDER_PATH"]
    readonly property string frameworksFolderPath: bundleSettingsProbe.xcodeSettings["FRAMEWORKS_FOLDER_PATH"]
    readonly property string javaFolderPath: bundleSettingsProbe.xcodeSettings["JAVA_FOLDER_PATH"]
    readonly property string localizedResourcesFolderPath: bundleSettingsProbe.xcodeSettings["LOCALIZED_RESOURCES_FOLDER_PATH"]
    readonly property string pluginsFolderPath: bundleSettingsProbe.xcodeSettings["PLUGINS_FOLDER_PATH"]
    readonly property string privateHeadersFolderPath: bundleSettingsProbe.xcodeSettings["PRIVATE_HEADERS_FOLDER_PATH"]
    readonly property string publicHeadersFolderPath: bundleSettingsProbe.xcodeSettings["PUBLIC_HEADERS_FOLDER_PATH"]
    readonly property string scriptsFolderPath: bundleSettingsProbe.xcodeSettings["SCRIPTS_FOLDER_PATH"]
    readonly property string sharedFrameworksFolderPath: bundleSettingsProbe.xcodeSettings["SHARED_FRAMEWORKS_FOLDER_PATH"]
    readonly property string sharedSupportFolderPath: bundleSettingsProbe.xcodeSettings["SHARED_SUPPORT_FOLDER_PATH"]
    readonly property string unlocalizedResourcesFolderPath: bundleSettingsProbe.xcodeSettings["UNLOCALIZED_RESOURCES_FOLDER_PATH"]
    readonly property string versionsFolderPath: bundleSettingsProbe.xcodeSettings["VERSIONS_FOLDER_PATH"]

    // private properties
    property string _productTypeIdentifier: Bundle.productTypeIdentifier(product.type)
    property stringList _productTypeIdentifierChain: bundleSettingsProbe.productTypeIdentifierChain

    property bool _useXcodeBuildSpecs: true // false to use ONLY the qbs build specs

    readonly property var extraEnv: ({
        "PRODUCT_BUNDLE_IDENTIFIER": identifier
    })

    readonly property var qmakeEnv: {
        return {
            "BUNDLEIDENTIFIER": identifier,
            "EXECUTABLE": product.targetName,
            "FULL_VERSION": product.version || "1.0", // CFBundleVersion
            "ICON": product.targetName, // ### QBS-73
            "LIBRARY": product.targetName,
            "SHORT_VERSION": product.version || "1.0", // CFBundleShortVersionString
            "TYPEINFO": signature // CFBundleSignature
        };
    }

    readonly property var defaultInfoPlist: {
        return {
            CFBundleDevelopmentRegion: "en", // default localization
            CFBundleDisplayName: product.targetName, // localizable
            CFBundleExecutable: product.targetName,
            CFBundleIdentifier: identifier,
            CFBundleInfoDictionaryVersion: "6.0",
            CFBundleName: product.targetName, // short display name of the bundle, localizable
            CFBundlePackageType: packageType,
            CFBundleShortVersionString: product.version || "1.0", // "release" version number, localizable
            CFBundleSignature: signature, // legacy creator code in Mac OS Classic, can be ignored
            CFBundleVersion: product.version || "1.0.0" // build version number, must be 3 octets
        };
    }

    validate: {
        if (!qbs.targetOS.contains("darwin"))
            return;
        if (!bundleSettingsProbe.found) {
            var error = "Bundle product type " + _productTypeIdentifier + " is not supported.";
            if ((_productTypeIdentifier || "").startsWith("com.apple.product-type."))
                error += " You may need to upgrade Xcode.";
            throw error;
        }

        var validator = new ModUtils.PropertyValidator("bundle");
        validator.setRequiredProperty("bundleName", bundleName);
        validator.setRequiredProperty("infoPlistPath", infoPlistPath);
        validator.setRequiredProperty("pbdevelopmentPlistPath", pbdevelopmentPlistPath);
        validator.setRequiredProperty("pkgInfoPath", pkgInfoPath);
        validator.setRequiredProperty("versionPlistPath", versionPlistPath);
        validator.setRequiredProperty("executablePath", executablePath);
        validator.setRequiredProperty("contentsFolderPath", contentsFolderPath);
        validator.setRequiredProperty("documentationFolderPath", documentationFolderPath);
        validator.setRequiredProperty("executableFolderPath", executableFolderPath);
        validator.setRequiredProperty("executablesFolderPath", executablesFolderPath);
        validator.setRequiredProperty("frameworksFolderPath", frameworksFolderPath);
        validator.setRequiredProperty("javaFolderPath", javaFolderPath);
        validator.setRequiredProperty("localizedResourcesFolderPath", localizedResourcesFolderPath);
        validator.setRequiredProperty("pluginsFolderPath", pluginsFolderPath);
        validator.setRequiredProperty("privateHeadersFolderPath", privateHeadersFolderPath);
        validator.setRequiredProperty("publicHeadersFolderPath", publicHeadersFolderPath);
        validator.setRequiredProperty("scriptsFolderPath", scriptsFolderPath);
        validator.setRequiredProperty("sharedFrameworksFolderPath", sharedFrameworksFolderPath);
        validator.setRequiredProperty("sharedSupportFolderPath", sharedSupportFolderPath);
        validator.setRequiredProperty("unlocalizedResourcesFolderPath", unlocalizedResourcesFolderPath);

        if (packageType === "FMWK") {
            validator.setRequiredProperty("frameworkVersion", frameworkVersion);
            validator.setRequiredProperty("versionsFolderPath", versionsFolderPath);
        }

        // extension and infoStringsPath might not be set
        return validator.validate();
    }

    FileTagger {
        fileTags: ["infoplist"]
        patterns: ["Info.plist", "*-Info.plist"]
    }

    Rule {
        condition: qbs.targetOS.contains("darwin")
        multiplex: true
        requiresInputs: false // TODO: The resources property should probably be a tag instead.
        inputs: ["infoplist", "partial_infoplist"]

        outputFileTags: ["bundle.input", "aggregate_infoplist"]
        outputArtifacts: {
            var artifacts = [];
            var embed = ModUtils.moduleProperty(product, "embedInfoPlist");
            if (ModUtils.moduleProperty(product, "isBundle") || embed) {
                artifacts.push({
                    filePath: FileInfo.joinPaths(
                                  product.destinationDirectory, product.name + "-Info.plist"),
                    fileTags: ["aggregate_infoplist"].concat(!embed ? ["bundle.input"] : []),
                    bundle: {
                        _bundleFilePath: FileInfo.joinPaths(
                                             product.destinationDirectory,
                                             ModUtils.moduleProperty(product, "infoPlistPath")),
                    }
                });
            }
            return artifacts;
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "generating Info.plist for " + product.name;
            cmd.highlight = "codegen";
            cmd.infoPlist = ModUtils.moduleProperty(product, "infoPlist") || {};
            cmd.processInfoPlist = ModUtils.moduleProperty(product, "processInfoPlist");
            cmd.infoPlistFormat = ModUtils.moduleProperty(product, "infoPlistFormat");
            cmd.extraEnv = ModUtils.moduleProperty(product, "extraEnv");
            cmd.qmakeEnv = ModUtils.moduleProperty(product, "qmakeEnv");

            cmd.buildEnv = product.moduleProperty("cpp", "buildEnv");

            cmd.developerPath = product.moduleProperty("xcode", "developerPath");
            cmd.platformInfoPlist = product.moduleProperty("xcode", "platformInfoPlist");
            cmd.sdkSettingsPlist = product.moduleProperty("xcode", "sdkSettingsPlist");
            cmd.toolchainInfoPlist = product.moduleProperty("xcode", "toolchainInfoPlist");

            cmd.osBuildVersion = product.moduleProperty("qbs", "hostOSBuildVersion");

            cmd.sourceCode = function() {
                var plist, process, key, i;

                // Contains the combination of default, file, and in-source keys and values
                // Start out with the contents of this file as the "base", if given
                var aggregatePlist = {};

                for (i = 0; i < (inputs.infoplist || []).length; ++i) {
                    aggregatePlist =
                            BundleTools.infoPlistContents(inputs.infoplist[i].filePath);
                    infoPlistFormat = (infoPlistFormat === "same-as-input")
                            ? BundleTools.infoPlistFormat(inputs.infoplist[i].filePath)
                            : "xml1";
                    break;
                }

                // Add local key-value pairs (overrides equivalent keys specified in the file if
                // one was given)
                for (key in infoPlist) {
                    if (infoPlist.hasOwnProperty(key))
                        aggregatePlist[key] = infoPlist[key];
                }

                // Do some postprocessing if desired
                if (processInfoPlist) {
                    // Add default values to the aggregate plist if the corresponding keys
                    // for those values are not already present
                    var defaultValues = ModUtils.moduleProperty(product, "defaultInfoPlist");
                    for (key in defaultValues) {
                        if (defaultValues.hasOwnProperty(key) && !(key in aggregatePlist))
                            aggregatePlist[key] = defaultValues[key];
                    }

                    // Add keys from platform's Info.plist if not already present
                    var platformInfo = {};
                    var sdkSettings = {};
                    var toolchainInfo = {};
                    if (developerPath) {
                        plist = new PropertyList();
                        try {
                            plist.readFromFile(platformInfoPlist);
                            platformInfo = plist.toObject();
                        } finally {
                            plist.clear();
                        }

                        var additionalProps = platformInfo["AdditionalInfo"];
                        for (key in additionalProps) {
                            // override infoPlist?
                            if (additionalProps.hasOwnProperty(key) && !(key in aggregatePlist))
                                aggregatePlist[key] = defaultValues[key];
                        }
                        props = platformInfo['OverrideProperties'];
                        for (key in props) {
                            aggregatePlist[key] = props[key];
                        }

                        plist = new PropertyList();
                        try {
                            plist.readFromFile(sdkSettingsPlist);
                            sdkSettings = plist.toObject();
                        } finally {
                            plist.clear();
                        }

                        plist = new PropertyList();
                        try {
                            plist.readFromFile(toolchainInfoPlist);
                            toolchainInfo = plist.toObject();
                        } finally {
                            plist.clear();
                        }
                    }

                    aggregatePlist["BuildMachineOSBuild"] = osBuildVersion;

                    // setup env
                    env = {
                        "SDK_NAME": sdkSettings["CanonicalName"],
                        "XCODE_VERSION_ACTUAL": toolchainInfo["DTXcode"],
                        "SDK_PRODUCT_BUILD_VERSION": toolchainInfo["DTPlatformBuild"],
                        "GCC_VERSION": platformInfo["DTCompiler"],
                        "XCODE_PRODUCT_BUILD_VERSION": platformInfo["DTPlatformBuild"],
                        "PLATFORM_PRODUCT_BUILD_VERSION": platformInfo["ProductBuildVersion"],
                    }
                    env["MAC_OS_X_PRODUCT_BUILD_VERSION"] = osBuildVersion;

                    for (key in extraEnv)
                        env[key] = extraEnv[key];

                    for (key in buildEnv)
                        env[key] = buildEnv[key];

                    for (key in qmakeEnv)
                        env[key] = qmakeEnv[key];

                    var expander = new DarwinTools.PropertyListVariableExpander();
                    expander.undefinedVariableFunction = function (key, varName) {
                        var msg = "Info.plist variable expansion encountered undefined variable '"
                                + varName + "' when expanding value for key '" + key
                                + "', defined in one of the following files:\n\t";
                        var allFilePaths = [];

                        for (i = 0; i < (inputs.infoplist || []).length; ++i)
                            allFilePaths.push(inputs.infoplist[i].filePath);

                        if (platformInfoPlist)
                            allFilePaths.push(platformInfoPlist);
                        msg += allFilePaths.join("\n\t") + "\n";
                        msg += "or in the bundle.infoPlist property of product '"
                                + product.name + "'";
                        console.warn(msg);
                    };
                    aggregatePlist = expander.expand(aggregatePlist, env);

                    // Add keys from partial Info.plists from asset catalogs, XIBs, and storyboards.
                    for (var j = 0; j < (inputs.partial_infoplist || []).length; ++j) {
                        var partialInfoPlist =
                                BundleTools.infoPlistContents(
                                    inputs.partial_infoplist[j].filePath)
                                || {};
                        for (key in partialInfoPlist) {
                            if (partialInfoPlist.hasOwnProperty(key))
                                aggregatePlist[key] = partialInfoPlist[key];
                        }
                    }

                }

                // Anything with an undefined or otherwise empty value should be removed
                // Only JSON-formatted plists can have null values, other formats error out
                // This also follows Xcode behavior
                DarwinTools.cleanPropertyList(aggregatePlist);

                if (infoPlistFormat === "same-as-input")
                    infoPlistFormat = "xml1";

                var validFormats = [ "xml1", "binary1", "json" ];
                if (!validFormats.contains(infoPlistFormat))
                    throw("Invalid Info.plist format " + infoPlistFormat + ". " +
                          "Must be in [xml1, binary1, json].");

                // Write the plist contents in the format appropriate for the current platform
                plist = new PropertyList();
                try {
                    plist.readFromObject(aggregatePlist);
                    plist.writeToFile(outputs.aggregate_infoplist[0].filePath, infoPlistFormat);
                } finally {
                    plist.clear();
                }
            }
            return cmd;
        }
    }

    Rule {
        condition: qbs.targetOS.contains("darwin")
        multiplex: true
        inputs: ["aggregate_infoplist"]

        outputFileTags: ["bundle.input", "pkginfo"]
        outputArtifacts: {
            var artifacts = [];
            if (ModUtils.moduleProperty(product, "isBundle") && ModUtils.moduleProperty(product, "generatePackageInfo")) {
                artifacts.push({
                    filePath: FileInfo.joinPaths(product.destinationDirectory, "PkgInfo"),
                    fileTags: ["bundle.input", "pkginfo"],
                    bundle: { _bundleFilePath: FileInfo.joinPaths(product.destinationDirectory, ModUtils.moduleProperty(product, "pkgInfoPath")) }
                });
            }
            return artifacts;
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "generating PkgInfo for " + product.name;
            cmd.highlight = "codegen";
            cmd.sourceCode = function() {
                var infoPlist = BundleTools.infoPlistContents(inputs.aggregate_infoplist[0].filePath);

                var pkgType = infoPlist['CFBundlePackageType'];
                if (!pkgType)
                    throw("CFBundlePackageType not found in Info.plist; this should not happen");

                var pkgSign = infoPlist['CFBundleSignature'];
                if (!pkgSign)
                    throw("CFBundleSignature not found in Info.plist; this should not happen");

                var pkginfo = new TextFile(outputs.pkginfo[0].filePath, TextFile.WriteOnly);
                pkginfo.write(pkgType + pkgSign);
                pkginfo.close();
            }
            return cmd;
        }
    }

    Rule {
        condition: qbs.targetOS.contains("darwin")
        multiplex: true
        inputs: ["bundle.input",
                 "aggregate_infoplist", "pkginfo", "hpp",
                 "icns", "xcent",
                 "compiled_ibdoc", "compiled_assetcatalog",
                 "xcode.provisioningprofile.main"]

        // Make sure the inputs of this rule are only those rules which produce outputs compatible
        // with the type of the bundle being produced.
        excludedInputs: Bundle.excludedAuxiliaryInputs(project, product)

        outputFileTags: [
            "bundle.content",
            "bundle.symlink.headers", "bundle.symlink.private-headers",
            "bundle.symlink.resources", "bundle.symlink.executable",
            "bundle.symlink.version", "bundle.hpp", "bundle.resource",
            "bundle.provisioningprofile", "bundle.content.copied", "bundle.application-executable"]
        outputArtifacts: {
            var i, artifacts = [];
            if (ModUtils.moduleProperty(product, "isBundle")) {
                for (i in inputs["bundle.input"]) {
                    var fp = inputs["bundle.input"][i].moduleProperty("bundle", "_bundleFilePath");
                    if (!fp)
                        throw("Artifact " + inputs["bundle.input"][i].filePath + " has no associated bundle file path");
                    var extraTags = inputs["bundle.input"][i].fileTags.contains("application")
                            ? ["bundle.application-executable"] : [];
                    artifacts.push({
                        filePath: fp,
                        fileTags: ["bundle.content", "bundle.content.copied"].concat(extraTags)
                    });
                }

                for (i in inputs["xcode.provisioningprofile.main"]) {
                    var ext = inputs["xcode.provisioningprofile.main"][i].fileName.split('.')[1];
                    artifacts.push({
                        filePath: FileInfo.joinPaths(product.destinationDirectory,
                                                     ModUtils.moduleProperty(product,
                                                                             "contentsFolderPath"),
                                                     "embedded." + ext),
                        fileTags: ["bundle.provisioningprofile", "bundle.content"]
                    });
                }

                var packageType = ModUtils.moduleProperty(product, "packageType");
                var isShallow = ModUtils.moduleProperty(product, "isShallow");
                if (packageType === "FMWK" && !isShallow) {
                    var publicHeaders = ModUtils.moduleProperty(product, "publicHeaders");
                    if (publicHeaders && publicHeaders.length) {
                        artifacts.push({
                            filePath: FileInfo.joinPaths(product.destinationDirectory, ModUtils.moduleProperty(product, "bundleName"), "Headers"),
                            fileTags: ["bundle.symlink.headers", "bundle.content"]
                        });
                    }

                    var privateHeaders = ModUtils.moduleProperty(product, "privateHeaders");
                    if (privateHeaders && privateHeaders.length) {
                        artifacts.push({
                            filePath: FileInfo.joinPaths(product.destinationDirectory, ModUtils.moduleProperty(product, "bundleName"), "PrivateHeaders"),
                            fileTags: ["bundle.symlink.private-headers", "bundle.content"]
                        });
                    }

                    artifacts.push({
                        filePath: FileInfo.joinPaths(product.destinationDirectory, ModUtils.moduleProperty(product, "bundleName"), "Resources"),
                        fileTags: ["bundle.symlink.resources", "bundle.content"]
                    });

                    artifacts.push({
                        filePath: FileInfo.joinPaths(product.destinationDirectory, ModUtils.moduleProperty(product, "bundleName"), product.targetName),
                        fileTags: ["bundle.symlink.executable", "bundle.content"]
                    });

                    artifacts.push({
                        filePath: FileInfo.joinPaths(product.destinationDirectory, ModUtils.moduleProperty(product, "versionsFolderPath"), "Current"),
                        fileTags: ["bundle.symlink.version", "bundle.content"]
                    });
                }

                var headerTypes = ["public", "private"];
                for (var h in headerTypes) {
                    var sources = ModUtils.moduleProperty(product, headerTypes[h] + "Headers");
                    var destination = FileInfo.joinPaths(product.destinationDirectory, ModUtils.moduleProperty(product, headerTypes[h] + "HeadersFolderPath"));
                    for (i in sources) {
                        artifacts.push({
                            filePath: FileInfo.joinPaths(destination, FileInfo.fileName(sources[i])),
                            fileTags: ["bundle.hpp", "bundle.content"]
                        });
                    }
                }

                sources = ModUtils.moduleProperty(product, "resources");
                for (i in sources) {
                    destination = BundleTools.destinationDirectoryForResource(product, {baseDir: FileInfo.path(sources[i]), fileName: FileInfo.fileName(sources[i])});
                    artifacts.push({
                        filePath: FileInfo.joinPaths(destination, FileInfo.fileName(sources[i])),
                        fileTags: ["bundle.resource", "bundle.content"]
                    });
                }

                var wrapperPath = FileInfo.joinPaths(
                            product.destinationDirectory,
                            ModUtils.moduleProperty(product, "bundleName"));
                for (var i = 0; i < artifacts.length; ++i)
                    artifacts[i].bundle = { wrapperPath: wrapperPath };
            }
            return artifacts;
        }

        prepare: {
            var i, cmd, commands = [];
            var packageType = ModUtils.moduleProperty(product, "packageType");

            var bundleType = "bundle";
            if (packageType === "APPL")
                bundleType = "application";
            if (packageType === "FMWK")
                bundleType = "framework";

            var bundles = outputs.bundle;
            for (i in bundles) {
                cmd = new Command("mkdir", ["-p", bundles[i].filePath]);
                cmd.description = "creating " + bundleType + " " + product.targetName;
                commands.push(cmd);

                cmd = new Command("touch", ["-c", bundles[i].filePath]);
                cmd.silent = true;
                commands.push(cmd);
            }

            // Product is unbundled
            if (commands.length === 0) {
                cmd = new JavaScriptCommand();
                cmd.silent = true;
                cmd.sourceCode = function () { };
                commands.push(cmd);
            }

            var symlinks = outputs["bundle.symlink.version"];
            for (i in symlinks) {
                cmd = new Command("ln", ["-sfn", ModUtils.moduleProperty(product, "frameworkVersion"),
                                  symlinks[i].filePath]);
                cmd.silent = true;
                commands.push(cmd);
            }

            var publicHeaders = outputs["bundle.symlink.headers"];
            for (i in publicHeaders) {
                cmd = new Command("ln", ["-sfn", "Versions/Current/Headers",
                                         publicHeaders[i].filePath]);
                cmd.silent = true;
                commands.push(cmd);
            }

            var privateHeaders = outputs["bundle.symlink.private-headers"];
            for (i in privateHeaders) {
                cmd = new Command("ln", ["-sfn", "Versions/Current/PrivateHeaders",
                                         privateHeaders[i].filePath]);
                cmd.silent = true;
                commands.push(cmd);
            }

            var resources = outputs["bundle.symlink.resources"];
            for (i in resources) {
                cmd = new Command("ln", ["-sfn", "Versions/Current/Resources",
                                         resources[i].filePath]);
                cmd.silent = true;
                commands.push(cmd);
            }

            var executables = outputs["bundle.symlink.executable"];
            for (i in executables) {
                cmd = new Command("ln", ["-sfn", FileInfo.joinPaths("Versions", "Current", product.targetName),
                                         executables[i].filePath]);
                cmd.silent = true;
                commands.push(cmd);
            }

            function sortedArtifactList(list, func) {
                if (list) {
                    return list.sort(func || (function (a, b) {
                        return a.filePath.localeCompare(b.filePath);
                    }));
                }
            }

            var bundleInputs = sortedArtifactList(inputs["bundle.input"], function (a, b) {
                return a.moduleProperty("bundle", "_bundleFilePath").localeCompare(
                            b.moduleProperty("bundle", "_bundleFilePath"));
            });
            var bundleContents = sortedArtifactList(outputs["bundle.content.copied"]);
            for (i in bundleContents) {
                cmd = new JavaScriptCommand();
                cmd.silent = true;
                cmd.source = bundleInputs[i].filePath;
                cmd.destination = bundleContents[i].filePath;
                cmd.sourceCode = function() {
                    File.copy(source, destination);
                };
                commands.push(cmd);
            }

            var provisioningProfiles = outputs["bundle.provisioningprofile"];
            for (i in provisioningProfiles) {
                cmd = new JavaScriptCommand();
                cmd.description = "copying provisioning profile";
                cmd.highlight = "filegen";
                cmd.source = inputs["xcode.provisioningprofile.main"][i].filePath;
                cmd.destination = provisioningProfiles[i].filePath;
                cmd.sourceCode = function() {
                    File.copy(source, destination);
                };
                commands.push(cmd);
            }

            cmd = new JavaScriptCommand();
            cmd.description = "copying public headers";
            cmd.highlight = "filegen";
            cmd.sources = ModUtils.moduleProperty(product, "publicHeaders");
            cmd.destination = FileInfo.joinPaths(product.destinationDirectory, ModUtils.moduleProperty(product, "publicHeadersFolderPath"));
            cmd.sourceCode = function() {
                var i;
                for (var i in sources) {
                    File.copy(sources[i], FileInfo.joinPaths(destination, FileInfo.fileName(sources[i])));
                }
            };
            if (cmd.sources && cmd.sources.length)
                commands.push(cmd);

            cmd = new JavaScriptCommand();
            cmd.description = "copying private headers";
            cmd.highlight = "filegen";
            cmd.sources = ModUtils.moduleProperty(product, "privateHeaders");
            cmd.destination = FileInfo.joinPaths(product.destinationDirectory, ModUtils.moduleProperty(product, "privateHeadersFolderPath"));
            cmd.sourceCode = function() {
                var i;
                for (var i in sources) {
                    File.copy(sources[i], FileInfo.joinPaths(destination, FileInfo.fileName(sources[i])));
                }
            };
            if (cmd.sources && cmd.sources.length)
                commands.push(cmd);

            cmd = new JavaScriptCommand();
            cmd.description = "copying resources";
            cmd.highlight = "filegen";
            cmd.sources = ModUtils.moduleProperty(product, "resources");
            cmd.sourceCode = function() {
                var i;
                for (var i in sources) {
                    var destination = BundleTools.destinationDirectoryForResource(product, {baseDir: FileInfo.path(sources[i]), fileName: FileInfo.fileName(sources[i])});
                    File.copy(sources[i], FileInfo.joinPaths(destination, FileInfo.fileName(sources[i])));
                }
            };
            if (cmd.sources && cmd.sources.length)
                commands.push(cmd);

            if (product.moduleProperty("qbs", "hostOS").contains("darwin")) {
                for (i in bundles) {
                    var actualSigningIdentity = product.moduleProperty("xcode", "actualSigningIdentity");
                    var codesignDisplayName = product.moduleProperty("xcode", "actualSigningIdentityDisplayName");
                    if (actualSigningIdentity) {
                        // If this is a framework, we need to sign its versioned directory
                        var subpath = "";
                        var frameworkVersion = ModUtils.moduleProperty(product, "frameworkVersion");
                        if (frameworkVersion) {
                            subpath = ModUtils.moduleProperty(product, "contentsFolderPath");
                            subpath = subpath.substring(subpath.indexOf(ModUtils.moduleProperty("qbs", "pathSeparator")));
                        }

                        var args = product.moduleProperty("xcode", "codesignFlags") || [];
                        args.push("--force");
                        args.push("--sign", actualSigningIdentity);
                        args = args.concat(DarwinTools._codeSignTimestampFlags(product));

                        for (var j in inputs.xcent) {
                            args.push("--entitlements", inputs.xcent[j].filePath);
                            break; // there should only be one
                        }
                        args.push(bundles[i].filePath + subpath);

                        cmd = new Command(product.moduleProperty("xcode", "codesignPath"), args);
                        cmd.description = "codesign "
                                + ModUtils.moduleProperty(product, "bundleName")
                                + " using " + codesignDisplayName
                                + " (" + actualSigningIdentity + ")";
                        commands.push(cmd);
                    }

                    if (bundleType === "application"
                            && product.moduleProperty("qbs", "targetOS").contains("macos")) {
                        cmd = new Command(ModUtils.moduleProperty(product, "lsregisterPath"),
                                          ["-f", bundles[i].filePath]);
                        cmd.description = "register " + ModUtils.moduleProperty(product, "bundleName");
                        commands.push(cmd);
                    }
                }
            }

            return commands;
        }
    }
}
