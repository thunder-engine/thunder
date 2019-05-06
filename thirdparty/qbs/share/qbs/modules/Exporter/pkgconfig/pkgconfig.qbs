import qbs.FileInfo
import qbs.TextFile

import "pkgconfig.js" as HelperFunctions

Module {
    property string fileName: product.targetName + ".pc"
    property bool autoDetect: true
    property var transformFunction // function(product, moduleName, propertyName, valueElement)
    property stringList excludedDependencies

    property string nameEntry: product.name
    property string descriptionEntry: product.name
    property string versionEntry: product.version
    property string urlEntry
    property stringList cflagsEntry: []
    property stringList libsEntry: []
    property stringList libsPrivateEntry: []
    property stringList requiresEntry: []
    property stringList requiresPrivateEntry: []
    property stringList conflictsEntry: []

    property var customVariables

    property bool _usePrefix: autoDetect && qbs.installPrefix

    additionalProductTypes: ["Exporter.pkgconfig.pc"]

    Rule {
        multiplex: true
        requiresInputs: false

        // Make sure all relevant library artifacts have been created by the time we run.
        auxiliaryInputs: {
            if (!autoDetect)
                return undefined;
            if (product.type.contains("staticlibrary"))
                return ["staticlibrary"];
            if (product.type.contains("dynamiclibrary"))
                return ["dynamiclibrary"];
        }

        Artifact {
            filePath: product.Exporter.pkgconfig.fileName
            fileTags: ["Exporter.pkgconfig.pc"]
        }
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.description = "Creating " + output.fileName;
            cmd.sourceCode = function() {
                var f = new TextFile(output.filePath, TextFile.WriteOnly);
                if (product.Exporter.pkgconfig._usePrefix)
                    f.writeLine("prefix=" + product.qbs.installPrefix + "\n");
                var customVariables = product.Exporter.pkgconfig.customVariables;
                if (customVariables) {
                    for (var customVar in customVariables)
                        f.writeLine(customVar + "=" + customVariables[customVar]);
                    f.writeLine("");
                }
                var autoDetectedData = HelperFunctions.collectAutodetectedData(product);
                HelperFunctions.writeEntry(product, f, "Name", "nameEntry", true);
                HelperFunctions.writeEntry(product, f, "Description", "descriptionEntry", true);
                HelperFunctions.writeEntry(product, f, "Version", "versionEntry", true);
                HelperFunctions.writeEntry(product, f, "URL", "urlEntry");
                HelperFunctions.writeEntry(product, f, "Cflags", "cflagsEntry", false,
                                           autoDetectedData.cflags);
                HelperFunctions.writeEntry(product, f, "Libs", "libsEntry", false,
                                           autoDetectedData.libs);
                HelperFunctions.writeEntry(product, f, "Libs.private", "libsPrivateEntry");
                HelperFunctions.writeEntry(product, f, "Requires", "requiresEntry", false,
                                           autoDetectedData.requires);
                HelperFunctions.writeEntry(product, f, "Requires.private", "requiresPrivateEntry",
                                           false, autoDetectedData.requiresPrivate);
                HelperFunctions.writeEntry(product, f, "Conflicts", "conflictsEntry");
            };
            return [cmd];
        }
    }

    validate: {
        if (requiresEntry && excludedDependencies
                && requiresEntry.containsAny(excludedDependencies)) {
            throw "The contents of Export.pkgconfig.requiresEntry and "
                    + "Export.pkgconfig.excludedDependencies must not overlap.";
        }
    }
}
