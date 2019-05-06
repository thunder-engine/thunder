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

var Environment = require("qbs.Environment");
var File = require("qbs.File");
var FileInfo = require("qbs.FileInfo");
var Process = require("qbs.Process");
var TemporaryDir = require("qbs.TemporaryDir");
var TextFile = require("qbs.TextFile");
var Utilities = require("qbs.Utilities");

function mergeCFiles(inputs, outputFilePath)
{
    var f = new TextFile(outputFilePath, TextFile.WriteOnly);
    try {
        for (var i = 0; i < inputs.length; ++i)
            f.writeLine('#include ' + Utilities.cStringQuote(inputs[i].filePath));
    } finally {
        f.close();
    }
}

function sanitizedList(list, product, fullPropertyName) {
    if (!Array.isArray(list))
        return list;
    var filterFunc = function(elem) {
        if (typeof elem === "string" && elem.length === 0) {
            var msg = "Removing empty string from value of property '" + fullPropertyName + "'";
            // product might actually be a module
            if (product.name)
                msg += " in product '" + product.name + "'.";
            console.warn(msg);
            return false;
        }
        return true;
    }
    return list.filter(filterFunc);
}

function checkCompatibilityMode(project, minimumQbsVersion, message) {
    if (Utilities.versionCompare(project.minimumQbsVersion || "1.3", minimumQbsVersion) < 0) {
        console.warn([message || "",
                      "This message can be silenced by setting your Project's " +
                      "minimumQbsVersion to " + minimumQbsVersion +
                      " (and the new behavior will take effect)."].join(" "));
        return true;
    }

    return false;
}

function artifactInstalledFilePath(artifact) {
    var relativeInstallDir = artifact.moduleProperty("qbs", "installDir");
    var installPrefix = artifact.moduleProperty("qbs", "installPrefix");
    var installSourceBase = artifact.moduleProperty("qbs", "installSourceBase");
    var targetDir = FileInfo.joinPaths(artifact.moduleProperty("qbs", "installRoot"),
                                       installPrefix, relativeInstallDir);
    if (installSourceBase) {
        if (!FileInfo.isAbsolutePath(installSourceBase))
            throw "installSourceBase is not an absolute path";
        if (!artifact.filePath.startsWith(installSourceBase))
            throw "artifact file path doesn't start with the value of qbs.installSourceBase";
        return FileInfo.joinPaths(targetDir, artifact.filePath.substr(installSourceBase.length + 1));
    }
    return FileInfo.joinPaths(targetDir, artifact.fileName);
}

/**
  * Given a list of file tags, returns the file tag (one of [c, cpp, objc, objcpp])
  * corresponding to the C-family language the file should be compiled as.
  *
  * If no such tag is found, undefined is returned. If more than one match is
  * found, an exception is thrown.
  */
function fileTagForTargetLanguage(fileTags) {
    var srcTags = ["c", "cpp", "objc", "objcpp", "asm", "asm_cpp"];
    var pchTags = ["c_pch", "cpp_pch", "objc_pch", "objcpp_pch"];

    var canonicalTag = undefined;
    var foundTagCount = 0;
    for (var i = 0; i < fileTags.length; ++i) {
        var idx = srcTags.indexOf(fileTags[i]);
        if (idx === -1)
            idx = pchTags.indexOf(fileTags[i]);

        if (idx !== -1) {
            canonicalTag = srcTags[idx];
            if (++foundTagCount > 1)
                break;
        }
    }

    if (foundTagCount > 1)
        throw ("source files cannot be identified as more than one language");

    return foundTagCount == 1 ? canonicalTag : undefined;
}

/**
  * Returns the name of a language-specific property given the file tag
  * for that property, and the base property name.
  *
  * If \a fileTag is undefined, the language-agnostic property name is returned.
  */
function languagePropertyName(propertyName, fileTag) {
    if (!fileTag)
        fileTag = "common";

    var asm = {
        "flags": "assemblerFlags",
        "platformFlags": "platformAssemblerFlags"
    };

    var map = {
        "c": {
            "flags": "cFlags",
            "platformFlags": "platformCFlags",
            "usePrecompiledHeader": "useCPrecompiledHeader"
        },
        "cpp": {
            "flags": "cxxFlags",
            "platformFlags": "platformCxxFlags",
            "usePrecompiledHeader": "useCxxPrecompiledHeader"
        },
        "objc": {
            "flags": "objcFlags",
            "platformFlags": "platformObjcFlags",
            "usePrecompiledHeader": "useObjcPrecompiledHeader"
        },
        "objcpp": {
            "flags": "objcxxFlags",
            "platformFlags": "platformObjcxxFlags",
            "usePrecompiledHeader": "useObjcxxPrecompiledHeader"
        },
        "common": {
            "flags": "commonCompilerFlags",
            "platformFlags": "platformCommonCompilerFlags"
        },
        "asm": asm,
        "asm_cpp": asm
    };

    var lang = map[fileTag];
    if (!lang)
        return propertyName;

    return lang[propertyName] || propertyName;
}

function modulePropertiesFromArtifacts(product, artifacts, moduleName, propertyName, langFilter) {
    var result = product.moduleProperty(
                moduleName, languagePropertyName(propertyName, langFilter)) || [];
    for (var i in artifacts) {
        var artifactProp = artifacts[i].moduleProperty(
                    moduleName, languagePropertyName(propertyName, langFilter));
        if (artifactProp)
            result = result.concat(artifactProp);
    }
    return sanitizedList(result, product, moduleName + "." + propertyName);
}

function moduleProperty(product, propertyName, langFilter)
{
    return sanitizedModuleProperty(product, product.moduleName, propertyName, langFilter);
}

function sanitizedModuleProperty(obj, moduleName, propertyName, langFilter) {
    return sanitizedList(obj.moduleProperty(moduleName,
                                            languagePropertyName(propertyName, langFilter)),
                         obj, moduleName + "." + propertyName);
}

/**
  * Returns roughly the same value as moduleProperty for a product, but ensures that all of the
  * given input artifacts share the same value of said property, as a sort of sanity check.
  *
  * This allows us to verify that users do not, for example, try to set different values on input
  * artifacts for which the value is input specific (not product specific), but which must be the
  * same for all inputs.
  */
function modulePropertyFromArtifacts(product, artifacts, moduleName, propertyName, langFilter) {
    var values = [product.moduleProperty(moduleName, languagePropertyName(propertyName, langFilter))];
    for (var i in artifacts) {
        var value = artifacts[i].moduleProperty(moduleName, languagePropertyName(propertyName, langFilter));
        if (!values.contains(value)) {
            values.push(value);
        }
    }

    if (values.length !== 1) {
        throw "The value of " + [moduleName, propertyName].join(".")
                + " must be identical for the following input artifacts: "
                + artifacts.map(function (artifact) { return artifact.filePath; });
    }

    return values[0];
}

function concatAll() {
    var result = [];
    for (var i = 0; i < arguments.length; ++i) {
        var arg = arguments[i];
        if (arg === undefined)
            continue;
        else if (arg instanceof Array)
            result = result.concat(arg);
        else
            result.push(arg);
    }
    return result;
}

function allFileTags(fileTaggers) {
    var tags = [];
    for (var ext in fileTaggers)
        tags = tags.uniqueConcat(fileTaggers[ext]);
    return tags;
}

/**
  * Flattens a dictionary (string keys to strings)
  * into a string list containing items like \c key=value1
  */
function flattenDictionary(dict, separator) {
    separator = separator || "=";
    var list = [];
    for (var i in dict) {
        var value = i;
        if (dict[i] !== undefined) // allow differentiation between undefined and empty string
            value += separator + dict[i];
        list.push(value);
    }
    return list;
}

function ModuleError(message) {
    var e = new Error(message);
    e.fileName = "";
    return e;
}

var EnvironmentVariable = (function () {
    function EnvironmentVariable(name, separator, convertPathSeparators) {
        if (!name)
            throw "EnvironmentVariable c'tor needs a name as first argument.";
        this.name = name;
        this.value = Environment.getEnv(name) || "";
        this.separator = separator || "";
        this.convertPathSeparators = convertPathSeparators || false;
    }
    EnvironmentVariable.prototype.prepend = function (v) {
        if (this.value.length > 0 && this.value.charAt(0) !== this.separator)
            this.value = this.separator + this.value;
        if (this.convertPathSeparators)
            v = FileInfo.toWindowsSeparators(v);
        this.value = v + this.value;
    };

    EnvironmentVariable.prototype.append = function (v) {
        if (this.value.length > 0)
            this.value += this.separator;
        if (this.convertPathSeparators)
            v = FileInfo.toWindowsSeparators(v);
        this.value += v;
    };

    EnvironmentVariable.prototype.set = function () {
        Environment.putEnv(this.name, this.value);
    };

    EnvironmentVariable.prototype.unset = function () {
        Environment.unsetEnv(this.name);
    };

    return EnvironmentVariable;
})();

var PropertyValidator = (function () {
    function PropertyValidator(moduleName) {
        this.requiredProperties = {};
        this.propertyValidators = [];
        if (!moduleName)
            throw "PropertyValidator c'tor needs a module name as a first argument.";
        this.moduleName = moduleName;
    }
    PropertyValidator.prototype.setRequiredProperty = function (propertyName, propertyValue, message) {
        this.requiredProperties[propertyName] = { propertyValue: propertyValue, message: message };
    };

    PropertyValidator.prototype.addRangeValidator = function (propertyName, propertyValue, min, max, allowFloats) {
        var message = [];
        if (min !== undefined)
            message.push(">= " + min);
        if (max !== undefined)
            message.push("<= " + max);

        this.addCustomValidator(propertyName, propertyValue, function (value) {
            if (typeof value !== "number")
                return false;
            if (!allowFloats && value % 1 !== 0)
                return false;
            if (min !== undefined && value < min)
                return false;
            if (max !== undefined && value > max)
                return false;
            return true;
        }, "must be " + (!allowFloats ? "an integer " : "") + message.join(" and "));
    };

    PropertyValidator.prototype.addVersionValidator = function (propertyName, propertyValue, minComponents, maxComponents, allowSuffixes) {
        if (minComponents !== undefined && (typeof minComponents !== "number" || minComponents % 1 !== 0 || minComponents < 1))
            throw "minComponents must be at least 1";
        if (maxComponents !== undefined && (typeof maxComponents !== "number" || maxComponents % 1 !== 0 || maxComponents < minComponents))
            throw "maxComponents must be >= minComponents";

        this.addCustomValidator(propertyName, propertyValue, function (value) {
            if (typeof value !== "string")
                return false;
            return value && value.match("^[0-9]+(\\.[0-9]+){" + ((minComponents - 1) || 0) + "," + ((maxComponents - 1) || "") + "}" + (!allowSuffixes ? "$" : "")) !== null;
        }, "must be a version number with " + (minComponents === maxComponents
                ? minComponents : (minComponents + " to " + maxComponents))
                  + (minComponents === maxComponents && minComponents === 1
                     ? " component" : " components"));
    };

    PropertyValidator.prototype.addFileNameValidator = function (propertyName, propertyValue) {
        this.addCustomValidator(propertyName, propertyValue, function (value) {
            return !/[/?<>\\:*|"\u0000-\u001f\u0080-\u009f]/.test(propertyValue)
                && propertyValue !== "." && propertyValue !== "..";
        }, "cannot contain reserved or control characters and cannot be \".\" or \"..\"");
    };

    PropertyValidator.prototype.addCustomValidator = function (propertyName, propertyValue, validator, message) {
        this.propertyValidators.push({
            propertyName: propertyName,
            propertyValue: propertyValue,
            validator: validator,
            message: message
        });
    };

    PropertyValidator.prototype.validate = function (throwOnError) {
        var i;
        var lines;

        // Find any missing properties
        var missingProperties = {};
        for (i in this.requiredProperties) {
            var propValue = this.requiredProperties[i].propertyValue;
            if (propValue === undefined || propValue === null || propValue === "") {
                missingProperties[i] = this.requiredProperties[i];
            }
        }

        // Find any properties that don't satisfy their validator function
        var invalidProperties = {};
        for (var j = 0; j < this.propertyValidators.length; ++j) {
            var v = this.propertyValidators[j];
            if (!v.validator(v.propertyValue)) {
                var messages = invalidProperties[v.propertyName] || [];
                messages.push(v.message);
                invalidProperties[v.propertyName] = messages;
            }
        }

        var errorMessage = "";
        if (Object.keys(missingProperties).length > 0) {
            errorMessage += "The following properties are not set. Set them in your profile or product:\n";
            lines = [];
            for (i in missingProperties) {
                var obj = missingProperties[i];
                lines.push(this.moduleName + "." + i + ((obj && obj.message) ? (": " + obj.message) : ""));
            }
            errorMessage += lines.join("\n");
        }

        if (Object.keys(invalidProperties).length > 0) {
            if (errorMessage)
                errorMessage += "\n";
            errorMessage += "The following properties have invalid values:\n";
            lines = [];
            for (i in invalidProperties) {
                for (j in invalidProperties[i]) {
                    lines.push(this.moduleName + "." + i + ": " + invalidProperties[i][j]);
                }
            }
            errorMessage += lines.join("\n");
        }

        if (throwOnError !== false && errorMessage.length > 0)
            throw errorMessage;

        return errorMessage.length == 0;
    };
    return PropertyValidator;
})();

var BlackboxOutputArtifactTracker = (function () {
    function BlackboxOutputArtifactTracker() {
    }
    BlackboxOutputArtifactTracker.prototype.artifacts = function (outputDirectory) {
        var process;
        var fakeOutputDirectory;
        try {
            fakeOutputDirectory = new TemporaryDir();
            if (!fakeOutputDirectory.isValid())
                throw "could not create temporary directory";
            process = new Process();
            if (this.commandEnvironmentFunction) {
                var env = this.commandEnvironmentFunction(fakeOutputDirectory.path());
                for (var key in env)
                    process.setEnv(key, env[key]);
            }
            process.exec(this.command, this.commandArgsFunction(fakeOutputDirectory.path()), true);
            var artifacts = [];
            if (this.fileTaggers) {
                var files = this.findFiles(fakeOutputDirectory.path());
                for (var i = 0; i < files.length; ++i)
                    artifacts.push(this.createArtifact(fakeOutputDirectory.path(), files[i]));
            }
            if (this.processStdOutFunction)
                artifacts = artifacts.concat(this.processStdOutFunction(process.readStdOut()));
            artifacts = this.fixArtifactPaths(artifacts, outputDirectory, fakeOutputDirectory.path());
            return artifacts;
        }
        finally {
            if (process)
                process.close();
            if (fakeOutputDirectory)
                fakeOutputDirectory.remove();
        }
    };
    BlackboxOutputArtifactTracker.prototype.createArtifact = function (root, filePath) {
        for (var ext in this.fileTaggers) {
            if (filePath.endsWith(ext)) {
                return {
                    filePath: filePath,
                    fileTags: this.fileTaggers[ext]
                };
            }
        }
        if (!this.defaultFileTags) {
            var relFilePath = (filePath.startsWith(root + '/') || filePath.startsWith(root + '\\'))
                    ? filePath.substring(root.length + 1)
                    : filePath;
            throw "BlackboxOutputArtifactTracker: no matching file taggers for path '"
                    + relFilePath + "'. Set defaultFileTags to an array of file tags to "
                    + "apply to files not tagged by the fileTaggers map, which was:\n"
                    + JSON.stringify(this.fileTaggers, undefined, 4);
        }
        return {
            filePath: filePath,
            fileTags: this.defaultFileTags
        };
    };
    BlackboxOutputArtifactTracker.prototype.findFiles = function (dir) {
        var fileList = File.directoryEntries(dir, File.Files).map(function (p) {
            return FileInfo.joinPaths(dir, p); });
        var dirList = File.directoryEntries(dir, File.Dirs | File.NoDotAndDotDot).map(function (p) {
            return FileInfo.joinPaths(dir, p); });
        for (var i = 0; i < dirList.length; ++i)
            fileList = fileList.concat(this.findFiles(dirList[i]));
        return fileList;
    };
    BlackboxOutputArtifactTracker.prototype.fixArtifactPaths = function (artifacts, realBasePath, fakeBasePath) {
        for (var i = 0; i < artifacts.length; ++i)
            artifacts[i].filePath = realBasePath
                + artifacts[i].filePath.substr(fakeBasePath.length);
        return artifacts;
    };
    return BlackboxOutputArtifactTracker;
})();

function hasAnyOf(m, tokens) {
    for (var i = 0; i < tokens.length; ++i) {
        if (m[tokens[i]] !== undefined)
            return true;
    }
}

function guessArchitecture(m) {
    var architecture;
    if (m) {
        // based on the search algorithm from qprocessordetection.h in qtbase
        var arm64Defs = ["_M_ARM64", "__aarch64__", "__ARM64__"];
        if (hasAnyOf(m, ["__arm__", "__TARGET_ARCH_ARM", "_M_ARM"].concat(arm64Defs))) {
            if (hasAnyOf(m, arm64Defs)) {
                architecture = "arm64";
            } else {
                architecture = "arm";

                var foundSubarch = false;
                for (var i = 8; i >= 4; --i) {
                    var codes = ["zk", "tej", "te", "t2"].concat([].concat.apply([],
                        new Array(26)).map(function(_, i) { return String.fromCharCode(122 - i); }));
                    for (var j = 0; j < codes.length; ++j) {
                        if (m["__ARM_ARCH_" + i + codes[j].toUpperCase() + "__"] !== undefined) {
                            architecture += "v" + i + codes[j].toLowerCase();
                            foundSubarch = true;
                            break;
                        }
                    }

                    if (i === 7 && m["_ARM_ARCH_7"] !== undefined) {
                        architecture += "v7";
                        foundSubarch = true;
                    }

                    if (foundSubarch)
                        break;
                }
            }
        } else if (hasAnyOf(m, ["__i386", "__i386__", "_M_IX86"])) {
            architecture = "x86";
        } else if (hasAnyOf(m, ["__x86_64", "__x86_64__", "__amd64", "_M_X64", "_M_AMD64"])) {
            architecture = "x86_64";
            if (hasAnyOf(m, ["__x86_64h", "__x86_64h__"]))
                architecture = "x86_64h";
        } else if (hasAnyOf(m, ["__ia64", "__ia64__", "_M_IA64"])) {
            architecture = "ia64";
        } else if (hasAnyOf(m, ["__mips", "__mips__", "_M_MRX000"])) {
            architecture = "mips";
            if (hasAnyOf(m, ["_MIPS_ARCH_MIPS64", "__mips64"]))
                architecture += "64";
        } else if (hasAnyOf(m, ["__ppc__", "__ppc", "__powerpc__",
                                "_ARCH_COM", "_ARCH_PWR", "_ARCH_PPC", "_M_MPPC", "_M_PPC"])) {
            architecture = "ppc";
            if (hasAnyOf(m, ["__ppc64__", "__powerpc64__", "__64BIT__"]))
                architecture += "64";
        } else if (hasAnyOf(m, ["__s390__"])) {
            if (hasAnyOf(m, ["__s390x__"]))
                architecture = "s390x";
        } else if (hasAnyOf(m, ["__sparc__"])) {
            architecture = "sparc";
            if (hasAnyOf(m, ["__sparc64__"]))
                architecture += "64";
        } else if (hasAnyOf(m, ["__AVR__"])) {
            architecture = "avr";
        } else if (hasAnyOf(m, ["__AVR32__"])) {
            architecture = "avr32";
        }
    }

    return Utilities.canonicalArchitecture(architecture);
}

function guessTargetPlatform(m) {
    if (m) {
        if (hasAnyOf(m, ["__ANDROID__", "ANDROID"]))
            return "android";
        if (hasAnyOf(m, ["__QNXNTO__"]))
            return "qnx";
        if (hasAnyOf(m, ["__INTEGRITY"]))
            return "integrity";
        if (hasAnyOf(m, ["__vxworks"]))
            return "vxworks";
        if (hasAnyOf(m, ["__APPLE__"]))
            return "darwin";
        if (hasAnyOf(m, ["WIN32", "_WIN32", "__WIN32__", "__NT__"]))
            return "windows";
        if (hasAnyOf(m, ["_AIX"]))
            return "aix";
        if (hasAnyOf(m, ["hpux", "__hpux"]))
            return "hpux";
        if (hasAnyOf(m, ["__sun", "sun"]))
            return "solaris";
        if (hasAnyOf(m, ["__linux__", "__linux"]))
            return "linux";
        if (hasAnyOf(m, ["__FreeBSD__", "__DragonFly__", "__FreeBSD_kernel__"]))
            return "freebsd";
        if (hasAnyOf(m, ["__NetBSD__"]))
            return "netbsd";
        if (hasAnyOf(m, ["__OpenBSD__"]))
            return "openbsd";
        if (hasAnyOf(m, ["__GNU__"]))
            return "hurd";
        if (hasAnyOf(m, ["__HAIKU__"]))
            return "haiku";
    }
}
