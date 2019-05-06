import qbs
import qbs.File
import qbs.FileInfo
import qbs.Probes
import "../protobufbase.qbs" as ProtobufBase
import "../protobuf.js" as HelperFunctions

ProtobufBase {
    property string includePath: includeProbe.path
    property string libraryPath: libraryProbe.path

    Depends { name: "cpp" }

    cpp.libraryPaths: [libraryPath]
    cpp.dynamicLibraries: qbs.targetOS.contains("unix") ? ["protobuf", "pthread"] : ["protobuf"]
    cpp.includePaths: [outputDir, includePath]

    Rule {
        inputs: ["protobuf.input"]
        outputFileTags: ["hpp", "cpp"]
        outputArtifacts: {
            return [
                HelperFunctions.cppArtifact(input.protobuf.cpp, product, input, "hpp", ".pb.h"),
                HelperFunctions.cppArtifact(input.protobuf.cpp, product, input, "cpp", ".pb.cc")
            ];
        }

        prepare: HelperFunctions.doPrepare(input.protobuf.cpp, product, input, outputs, "cpp")
    }

    validate: {
        baseValidate();
        if (!HelperFunctions.checkPath(includePath))
            throw "Can't find cpp protobuf include files. Please set the includePath property.";
        if (!HelperFunctions.checkPath(libraryPath))
            throw "Can't find cpp protobuf library. Please set the libraryPath property.";
    }

    Probes.IncludeProbe {
        id: includeProbe
        names: "google/protobuf/message.h"
    }

    Probes.LibraryProbe {
        id: libraryProbe
        names: "protobuf"
    }
}
