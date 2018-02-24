import qbs
import "lexyacc.js" as HelperFunctions

Module {
    Depends { name: "cpp" }

    property bool enableCompilerWarnings: false
    property string lexBinary: "lex"
    property string yaccBinary: "yacc"
    property string outputTag: "c"
    property bool uniqueSymbolPrefix: false
    property stringList lexFlags: []
    property stringList yaccFlags: []

    readonly property string outputDir: product.buildDirectory + "/lexyacc"

    Rule {
        inputs: ["lex.input"]
        Artifact {
            filePath: HelperFunctions.outputFilePath(product, input, "lex.yy.c", false)
            fileTags: [product.moduleProperty("lex_yacc", "outputTag")]
            cpp.includePaths: (product.moduleProperty("cpp", "includePaths") || [])
                .concat([product.moduleProperty("lex_yacc", "outputDir")])
            cpp.warningLevel: input.moduleProperty("lex_yacc", "enableCompilerWarnings")
                              ? "all" : "none"
        }
        prepare: {
            var args = product.moduleProperty("lex_yacc", "lexFlags");
            if (product.moduleProperty("lex_yacc", "uniqueSymbolPrefix"))
                args.push("-P" + input.baseName, "-o" + output.filePath);
            args.push(input.filePath);
            var cmd = new Command(product.moduleProperty("lex_yacc", "lexBinary"), args);
            cmd.workingDirectory = product.moduleProperty("lex_yacc", "outputDir");
            cmd.description = "generating " + output.fileName;
            return [cmd];
        }
    }

    Rule {
        inputs: ["yacc.input"]
        Artifact {
            filePath: HelperFunctions.outputFilePath(product, input, "y.tab.c", true)
            fileTags: [product.moduleProperty("lex_yacc", "outputTag")]
            cpp.warningLevel: input.moduleProperty("lex_yacc", "enableCompilerWarnings")
                              ? "all" : "none"
        }
        Artifact {
            filePath: HelperFunctions.outputFilePath(product, input, "y.tab.h", true)
            fileTags: ["hpp"]
        }
        prepare: {
            var args = product.moduleProperty("lex_yacc", "yaccFlags");
            args.push("-d");
            if (product.moduleProperty("lex_yacc", "uniqueSymbolPrefix")) {
                args.push("-b", input.baseName, "-p", input.baseName);
            }
            args.push(input.filePath);
            var cmd = new Command(product.moduleProperty("lex_yacc", "yaccBinary"), args);
            cmd.workingDirectory = product.moduleProperty("lex_yacc", "outputDir");
            cmd.description = "generating "
                    + outputs[product.moduleProperty("lex_yacc", "outputTag")][0].fileName
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
