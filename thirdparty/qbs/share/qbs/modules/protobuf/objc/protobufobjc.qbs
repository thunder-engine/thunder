import qbs
import qbs.File
import qbs.FileInfo
import qbs.Probes
import "../protobufbase.qbs" as ProtobufBase
import "../protobuf.js" as HelperFunctions

ProtobufBase {
    property string includePath: includeProbe.path
    property string libraryPath: libraryProbe.path
    property string frameworkPath: frameworkProbe.path

    Depends { name: "cpp" }

    // library build
    Properties {
        condition: !frameworkPath
        cpp.includePaths: [outputDir, includePath]
        cpp.libraryPaths: [libraryPath]
        cpp.frameworks: ["Foundation"]
        cpp.dynamicLibraries: ["ProtocolBuffers"]
    }

    // framework build
    Properties {
        condition: frameworkPath
        cpp.includePaths: [outputDir]
        cpp.frameworkPaths: [frameworkPath]
        cpp.frameworks: ["Foundation", "Protobuf"]
        cpp.defines: ["GPB_USE_PROTOBUF_FRAMEWORK_IMPORTS"]
    }

    Rule {
        inputs: ["protobuf.input"]
        outputFileTags: ["hpp", "objc"]
        outputArtifacts: {
            return [
                HelperFunctions.objcArtifact(input.protobuf.objc, product, input, "hpp", ".pbobjc.h"),
                HelperFunctions.objcArtifact(input.protobuf.objc, product, input, "objc", ".pbobjc.m")
            ];
        }

        prepare: HelperFunctions.doPrepare(input.protobuf.objc, product, input, outputs, "objc")
    }

    validate: {
        baseValidate();
        if (!HelperFunctions.checkPath(frameworkPath)) {
            if (!HelperFunctions.checkPath(includePath))
                throw "Can't find objective-c protobuf include files. Please set the includePath or frameworkPath property.";
            if (!HelperFunctions.checkPath(libraryPath))
                throw "Can't find objective-c protobuf library. Please set the libraryPath or frameworkPath property.";
        }
    }

    Probes.IncludeProbe {
        id: includeProbe
        names: "GPBMessage.h"
    }

    Probes.LibraryProbe {
        id: libraryProbe
        names: "ProtocolBuffers"
    }

    Probes.FrameworkProbe {
        id: frameworkProbe
        names: ["Protobuf"]
    }
}
