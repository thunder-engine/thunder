import "lexyacc.js" as HelperFunctions

Module {
    Depends { name: "cpp" }

    property bool enableCompilerWarnings: false
    property string lexBinary: "lex"
    property string yaccBinary: "yacc"
    property string outputTag: "c"
    property bool uniqueSymbolPrefix: false
    property string lexOutputFilePath
    property string yaccOutputFilePath
    property stringList lexFlags: []
    property stringList yaccFlags: []

    readonly property string outputDir: product.buildDirectory + "/lexyacc"

    Rule {
        inputs: ["lex.input"]
        outputFileTags: [product.lex_yacc.outputTag]
        outputArtifacts: {
            var output = {
                fileTags: [product.lex_yacc.outputTag],
                lex_yacc: {},
            };
            var options = HelperFunctions.readLexOptions(input.filePath);
            if (!options.outfile && input.lex_yacc.lexOutputFilePath) {
                options.outfile = input.lex_yacc.lexOutputFilePath;
                output.lex_yacc.useOutfileFromModule = true;
            }
            output.filePath = HelperFunctions.lexOutputFilePath(input, "lex.yy.c", options);
            output.cpp = {
                includePaths: [].concat(input.cpp.includePaths, input.lex_yacc.outputDir),
                warningLevel: input.lex_yacc.enableCompilerWarnings ? "all" : "none",
            };
            return [output];
        }
        prepare: {
            var args = input.lex_yacc.lexFlags;
            if (output.lex_yacc.useOutfileFromModule)
                args.push("-o" + input.lex_yacc.lexOutputFilePath);
            else if (input.lex_yacc.uniqueSymbolPrefix)
                args.push("-P" + input.baseName, "-o" + output.filePath);
            args.push(input.filePath);
            var cmd = new Command(input.lex_yacc.lexBinary, args);
            cmd.workingDirectory = input.lex_yacc.outputDir;
            cmd.description = "generating " + output.fileName;
            return [cmd];
        }
    }

    Rule {
        inputs: ["yacc.input"]
        outputFileTags: [product.lex_yacc.outputTag, "hpp"]
        outputArtifacts: {
            var src = {
                fileTags: [product.lex_yacc.outputTag],
                lex_yacc: {},
            };
            var options = HelperFunctions.readYaccOptions(input.filePath);
            if (!options.output && input.lex_yacc.yaccOutputFilePath) {
                options.output = input.lex_yacc.yaccOutputFilePath;
                src.lex_yacc.useOutputFromModule = true;
            }
            var hdr = {
                filePath: HelperFunctions.yaccOutputFilePath(input, "y.tab.h", options),
                fileTags: ["hpp"],
            };
            src.filePath = HelperFunctions.yaccOutputFilePath(input, "y.tab.c", options);
            src.cpp = {
                includePaths: [].concat(input.cpp.includePaths, input.lex_yacc.outputDir),
                warningLevel: input.lex_yacc.enableCompilerWarnings ? "all" : "none",
            };
            return [hdr, src];
        }
        prepare: {
            var args = input.lex_yacc.yaccFlags;
            args.push("-d");
            var impl = outputs[input.lex_yacc.outputTag][0];
            if (impl.lex_yacc.useOutputFromModule)
                args.push("-o" + input.lex_yacc.yaccOutputFilePath);
            else if (input.lex_yacc.uniqueSymbolPrefix)
                args.push("-b", input.baseName, "-p", input.baseName);
            args.push(input.filePath);
            var cmd = new Command(input.lex_yacc.yaccBinary, args);
            cmd.workingDirectory = input.lex_yacc.outputDir;
            cmd.description = "generating "
                    + impl.fileName
                    + " and " + outputs["hpp"][0].fileName;
            return [cmd];
        }
    }

    FileTagger {
        patterns: ["*.l"]
        fileTags: ["lex.input"];
    }
    FileTagger {
        patterns: ["*.y"]
        fileTags: ["yacc.input"];
    }
}
