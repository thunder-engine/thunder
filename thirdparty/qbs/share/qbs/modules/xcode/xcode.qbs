import qbs.BundleTools
import qbs.Environment
import qbs.File
import qbs.FileInfo
import qbs.DarwinTools
import qbs.ModUtils
import qbs.Probes
import qbs.PropertyList
import 'xcode.js' as Xcode

Module {
    id: xcodeModule

    Probes.XcodeProbe {
        id: xcodeProbe
        developerPath: xcodeModule.developerPath
        platformType: xcodeModule.platformType
        platformPath: xcodeModule.platformPath
        devicePlatformPath: xcodeModule.devicePlatformPath
        xcodebuildPath: xcodeModule.xcodebuildPath
        sdksPath: xcodeModule.sdksPath
    }

    condition: qbs.targetOS.contains("darwin") &&
               qbs.toolchain && qbs.toolchain.contains("xcode")

    version: xcodeProbe.xcodeVersion

    property path developerPath: "/Applications/Xcode.app/Contents/Developer"
    property string sdk: DarwinTools.applePlatformName(qbs.targetOS, platformType)
    property stringList targetDevices: DarwinTools.targetDevices(qbs.targetOS)

    property string platformType: {
        if (qbs.targetOS.containsAny(["ios-simulator", "tvos-simulator", "watchos-simulator"]))
            return "simulator";
        if (qbs.targetOS.containsAny(["ios", "tvos", "watchos"]))
            return "device";
    }

    readonly property string sdkName: {
        if (_sdkSettings) {
            return _sdkSettings["CanonicalName"];
        }
    }

    readonly property string sdkVersion: {
        if (_sdkSettings) {
            return _sdkSettings["Version"];
        }
    }

    readonly property string latestSdkName: {
        if (_latestSdk) {
            return _latestSdk["CanonicalName"];
        }
    }

    readonly property string latestSdkVersion: {
        if (_latestSdk) {
            return _latestSdk["Version"];
        }
    }

    readonly property stringList availableSdkNames: {
        if (_availableSdks) {
            return _availableSdks.map(function (obj) { return obj["CanonicalName"]; });
        }
    }

    readonly property stringList availableSdkVersions: {
        if (_availableSdks) {
            return _availableSdks.map(function (obj) { return obj["Version"]; });
        }
    }

    property string signingIdentity
    readonly property string actualSigningIdentity: {
        if (_actualSigningIdentity && _actualSigningIdentity.length === 1)
            return _actualSigningIdentity[0][0];
    }

    readonly property string actualSigningIdentityDisplayName: {
        if (_actualSigningIdentity && _actualSigningIdentity.length === 1)
            return _actualSigningIdentity[0][1];
    }

    property string signingTimestamp: "none"

    property string provisioningProfile

    property string xcodebuildName: "xcodebuild"
    property string xcodebuildPath: FileInfo.joinPaths(developerPath, "usr", "bin", xcodebuildName)

    property string securityName: "security"
    property string securityPath: securityName

    property string codesignName: "codesign"
    property string codesignPath: codesignName
    property stringList codesignFlags

    readonly property path toolchainPath: FileInfo.joinPaths(toolchainsPath,
                                                             "XcodeDefault" + ".xctoolchain")
    readonly property path platformPath: FileInfo.joinPaths(platformsPath,
                                                            DarwinTools.applePlatformDirectoryName(
                                                                qbs.targetOS, platformType)
                                                            + ".platform")
    readonly property path devicePlatformPath: FileInfo.joinPaths(
                                                   platformsPath,
                                                   DarwinTools.applePlatformDirectoryName(
                                                       qbs.targetOS, "device")
                                                   + ".platform")
    readonly property path simulatorPlatformPath: FileInfo.joinPaths(
                                                      platformsPath,
                                                      DarwinTools.applePlatformDirectoryName(
                                                          qbs.targetOS, "simulator")
                                                      + ".platform")
    readonly property path sdkPath: FileInfo.joinPaths(sdksPath,
                                                       DarwinTools.applePlatformDirectoryName(
                                                           qbs.targetOS, platformType, sdkVersion)
                                                       + ".sdk")

    // private properties
    readonly property path toolchainsPath: FileInfo.joinPaths(developerPath, "Toolchains")
    readonly property path platformsPath: FileInfo.joinPaths(developerPath, "Platforms")
    readonly property path sdksPath: FileInfo.joinPaths(platformPath, "Developer", "SDKs")

    readonly property path platformInfoPlist: FileInfo.joinPaths(platformPath, "Info.plist")
    readonly property path sdkSettingsPlist: FileInfo.joinPaths(sdkPath, "SDKSettings.plist")
    readonly property path toolchainInfoPlist: FileInfo.joinPaths(toolchainPath,
                                                                  "ToolchainInfo.plist")

    readonly property stringList _actualSigningIdentity: {
        if (/^[A-Fa-f0-9]{40}$/.test(signingIdentity)) {
            return signingIdentity;
        }

        var identities = Xcode.findSigningIdentities(securityPath, signingIdentity);
        if (identities && identities.length > 1) {
            throw "Signing identity '" + signingIdentity + "' is ambiguous";
        }

        return identities;
    }

    property path provisioningProfilesPath: {
        return FileInfo.joinPaths(Environment.getEnv("HOME"), "Library/MobileDevice/Provisioning Profiles");
    }

    readonly property stringList standardArchitectures: _architectureSettings["ARCHS_STANDARD"]

    readonly property var _architectureSettings: xcodeProbe.architectureSettings

    readonly property var _availableSdks: xcodeProbe.availableSdks

    readonly property var _latestSdk: _availableSdks[_availableSdks.length - 1]

    readonly property var _sdkSettings: {
        if (_availableSdks) {
            for (var i in _availableSdks) {
                if (_availableSdks[i]["Version"] === sdk)
                    return _availableSdks[i];
                if (_availableSdks[i]["CanonicalName"] === sdk)
                    return _availableSdks[i];
            }

            // Latest SDK available for the platform
            if (DarwinTools.applePlatformName(qbs.targetOS, platformType) === sdk)
                return _latestSdk;
        }
    }

    qbs.sysroot: sdkPath

    validate: {
        if (!_availableSdks) {
            throw "There are no SDKs available for this platform in the Xcode installation.";
        }

        if (!_sdkSettings) {
            throw "There is no matching SDK available for " + sdk + ".";
        }

        var validator = new ModUtils.PropertyValidator("xcode");
        validator.setRequiredProperty("developerPath", developerPath);
        validator.setRequiredProperty("sdk", sdk);
        validator.setRequiredProperty("sdkName", sdkName);
        validator.setRequiredProperty("sdkVersion", sdkVersion);
        validator.setRequiredProperty("toolchainsPath", toolchainsPath);
        validator.setRequiredProperty("toolchainPath", toolchainPath);
        validator.setRequiredProperty("platformsPath", platformsPath);
        validator.setRequiredProperty("platformPath", platformPath);
        validator.setRequiredProperty("sdksPath", sdkPath);
        validator.setRequiredProperty("sdkPath", sdkPath);
        validator.addVersionValidator("sdkVersion", sdkVersion, 2, 2);
        validator.addCustomValidator("sdkName", sdkName, function (value) {
            return value === DarwinTools.applePlatformDirectoryName(
                        qbs.targetOS, platformType, sdkVersion, false).toLowerCase();
        }, "is '" + sdkName + "', but target OS is [" + qbs.targetOS.join(",")
        + "] and Xcode SDK version is '" + sdkVersion + "'");
        validator.addCustomValidator("sdk", sdk, function (value) {
            return value === sdkName || (value + sdkVersion) === sdkName;
        }, "is '" + sdk + "', but canonical SDK name is '" + sdkName + "'");
        validator.validate();
    }

    property var buildEnv: {
        var env = {
            "DEVELOPER_DIR": developerPath,
            "SDKROOT": sdkPath
        };

        var prefixes = [platformPath + "/Developer", toolchainPath, developerPath];
        for (var i = 0; i < prefixes.length; ++i) {
            var codesign_allocate = prefixes[i] + "/usr/bin/codesign_allocate";
            if (File.exists(codesign_allocate)) {
                env["CODESIGN_ALLOCATE"] = codesign_allocate;
                break;
            }
        }

        return env;
    }

    setupBuildEnvironment: {
        var v = new ModUtils.EnvironmentVariable("PATH", product.qbs.pathListSeparator, false);
        v.prepend(product.xcode.platformPath + "/Developer/usr/bin");
        v.prepend(product.xcode.developerPath + "/usr/bin");
        v.set();

        for (var key in product.xcode.buildEnv) {
            v = new ModUtils.EnvironmentVariable(key);
            v.value = product.xcode.buildEnv[key];
            v.set();
        }
    }

    Group {
        name: "Provisioning Profiles"
        prefix: xcode.provisioningProfilesPath + "/"
        files: ["*.mobileprovision", "*.provisionprofile"]
        fileTags: [] // HACK: provisioning profile handling is not yet ready and can break autotests
    }

    FileTagger {
        fileTags: ["xcode.entitlements"]
        patterns: ["*.entitlements"]
    }

    FileTagger {
        fileTags: ["xcode.provisioningprofile"]
        patterns: ["*.mobileprovision", "*.provisionprofile"]
    }

    Rule {
        inputs: ["xcode.provisioningprofile"]

        Artifact {
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         "provisioning-profiles",
                                         input.fileName + ".xml")
            fileTags: ["xcode.provisioningprofile.data"]
        }

        prepare: {
            var cmds = [];

            var cmd = new Command("openssl", ["smime", "-verify", "-noverify", "-inform", "DER",
                                  "-in", input.filePath, "-out", output.filePath]);
            cmd.silent = true;
            cmd.stderrFilterFunction = function (output) {
                return output.replace("Verification successful\n", "");
            };
            cmds.push(cmd);

            cmd = new JavaScriptCommand();
            cmd.silent = true;
            cmd.inputFilePath = input.filePath;
            cmd.outputFilePath = output.filePath;
            cmd.sourceCode = function() {
                var propertyList = new PropertyList();
                try {
                    propertyList.readFromFile(outputFilePath);
                    propertyList.readFromObject({
                        data: propertyList.toObject(),
                        fileName: FileInfo.fileName(inputFilePath),
                        filePath: inputFilePath
                    });
                    propertyList.writeToFile(outputFilePath, "xml1");
                } finally {
                    propertyList.clear();
                }
            };
            cmds.push(cmd);

            return cmds;
        }
    }

    Rule {
        multiplex: true
        inputs: ["xcode.provisioningprofile.data"]
        outputFileTags: ["xcode.provisioningprofile.main", "xcode.provisioningprofile.data.main"]

        outputArtifacts: {
            var artifacts = [];
            for (var i = 0; i < inputs["xcode.provisioningprofile.data"].length; ++i) {
                var dataFile = inputs["xcode.provisioningprofile.data"][i].filePath;
                var query = product.moduleProperty("xcode", "provisioningProfile");
                var obj = Xcode.provisioningProfilePlistContents(dataFile);
                if (obj.data && (obj.data.UUID === query || obj.data.Name === query)) {
                    console.log("Using provisioning profile: " + obj.filePath);

                    artifacts.push({
                        filePath: FileInfo.joinPaths(product.destinationDirectory, obj.fileName),
                        fileTags: ["xcode.provisioningprofile.main", obj.filePath]
                    });

                    artifacts.push({
                        filePath: FileInfo.joinPaths(product.destinationDirectory, obj.fileName + ".xml"),
                        fileTags: ["xcode.provisioningprofile.data.main", dataFile]
                    });
                }
            }
            return artifacts;
        }

        prepare: {
            var cmds = [];
            for (var tag in outputs) {
                for (var i = 0; i < outputs[tag].length; ++i) {
                    var output = outputs[tag][i];
                    var cmd = new JavaScriptCommand();
                    cmd.silent = true;
                    cmd.inputFilePath = output.fileTags.filter(function(f) { return f.startsWith('/'); })[0] // QBS-754
                    cmd.outputFilePath = output.filePath;
                    cmd.sourceCode = function() {
                        File.copy(inputFilePath, outputFilePath);
                    };
                    cmds.push(cmd);
                }
            }
            return cmds;
        }
    }

    Rule {
        inputs: ["xcode.entitlements", "xcode.provisioningprofile.data.main"]

        Artifact {
            filePath: FileInfo.joinPaths(product.destinationDirectory,
                                         product.targetName + ".xcent")
            fileTags: ["xcent", "bundle.input"]
        }

        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "generating entitlements";
            cmd.highlight = "codegen";
            cmd.bundleIdentifier = product.moduleProperty("bundle", "identifier");
            cmd.signingEntitlements = inputs["xcode.entitlements"].map(function (a) { return a.filePath; });
            cmd.platformPath = ModUtils.moduleProperty(product, "platformPath");
            cmd.sdkPath = ModUtils.moduleProperty(product, "sdkPath");
            cmd.sourceCode = function() {
                var i;
                var provData = Xcode.provisioningProfilePlistContents(input.filePath);
                if (provData)
                    provData = provData.data;

                var aggregateEntitlements = {};

                // Start building up an aggregate entitlements plist from the files in the SDKs,
                // which contain placeholders in the same manner as Info.plist
                function entitlementsFileContents(path) {
                    return File.exists(path) ? BundleTools.infoPlistContents(path) : undefined;
                }
                var entitlementsSources = [
                    entitlementsFileContents(FileInfo.joinPaths(platformPath, "Entitlements.plist")),
                    entitlementsFileContents(FileInfo.joinPaths(sdkPath, "Entitlements.plist"))
                ];

                for (i = 0; i < signingEntitlements.length; ++i) {
                    entitlementsSources.push(entitlementsFileContents(signingEntitlements[i]));
                }

                for (i = 0; i < entitlementsSources.length; ++i) {
                    var contents = entitlementsSources[i];
                    for (var key in contents) {
                        if (contents.hasOwnProperty(key))
                            aggregateEntitlements[key] = contents[key];
                    }
                }

                contents = provData["Entitlements"];
                for (key in contents) {
                    if (contents.hasOwnProperty(key) && !aggregateEntitlements.hasOwnProperty(key))
                        aggregateEntitlements[key] = contents[key];
                }

                // Expand entitlements variables with data from the provisioning profile
                var env = {
                    "AppIdentifierPrefix": provData["ApplicationIdentifierPrefix"] + ".",
                    "CFBundleIdentifier": bundleIdentifier
                };
                DarwinTools.expandPlistEnvironmentVariables(aggregateEntitlements, env, true);

                // Anything with an undefined or otherwise empty value should be removed
                // Only JSON-formatted plists can have null values, other formats error out
                // This also follows Xcode behavior
                DarwinTools.cleanPropertyList(aggregateEntitlements);

                var plist = new PropertyList();
                try {
                    plist.readFromObject(aggregateEntitlements);
                    plist.writeToFile(outputs.xcent[0].filePath, "xml1");
                } finally {
                    plist.clear();
                }
            };
            return [cmd];
        }
    }
}
