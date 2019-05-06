import qbs.File
import qbs.FileInfo
import qbs.ModUtils
import qbs.Probes
import qbs.Process
import qbs.Utilities
import "ico.js" as IcoUtils

Module {
    Probes.BinaryProbe {
        id: icotoolProbe
        names: ["icotool"]
    }

    Probes.IcoUtilsVersionProbe {
        id: icotoolVersionProbe
        toolFilePath: icotoolFilePath
    }

    version: icotoolVersionProbe.version

    property int alphaThreshold
    property int cursorHotspotX
    property int cursorHotspotY
    property bool raw

    // private properties
    property string icotoolFilePath: icotoolProbe.filePath
    readonly property bool hasCursorHotspotBug: Utilities.versionCompare(version, "0.32") < 0

    FileTagger {
        patterns: ["*.png"]
        fileTags: ["png"]
    }

    FileTagger {
        patterns: ["*.iconset"] // bundle
        fileTags: ["iconset"]
    }

    Rule {
        inputs: ["iconset"]

        Artifact {
            filePath: input.baseName + ".ico"
            fileTags: ["ico"]
        }

        prepare: IcoUtils.prepareIconset.apply(IcoUtils, arguments)
    }

    Rule {
        multiplex: true
        inputs: ["png"]

        Artifact {
            filePath: product.targetName + ".ico"
            fileTags: ["ico"]
        }

        prepare: IcoUtils.prepare.apply(IcoUtils, arguments)
    }

    Rule {
        inputs: ["iconset"]

        Artifact {
            filePath: input.baseName + ".cur"
            fileTags: ["cur"]
        }

        prepare: IcoUtils.prepareIconset.apply(IcoUtils, arguments)
    }

    Rule {
        multiplex: true
        inputs: ["png"]

        Artifact {
            filePath: product.targetName + ".cur"
            fileTags: ["cur"]
        }

        prepare: IcoUtils.prepare.apply(IcoUtils, arguments)
    }

    validate: {
        if (!icotoolFilePath)
            throw ModUtils.ModuleError("Could not find icotool in any of the following "
                                       + "locations:\n\t" + icotoolProbe.candidatePaths.join("\n\t")
                                       + "\nInstall the icoutils package on your platform.");

        if (!version)
            throw ModUtils.ModuleError("Could not determine icoutils package version.");

        var validator = new ModUtils.PropertyValidator("ico");
        validator.setRequiredProperty("version", version);
        validator.addVersionValidator("version", version, 2, 3);
        return validator.validate()
    }
}
