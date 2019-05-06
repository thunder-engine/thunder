import qbs.TextFile

Module {
    property var dict: ({})
    property string outputTag: "text"
    property string outputFileName
    FileTagger {
        patterns: ["*.in"]
        fileTags: ["texttemplate.input"]
    }
    Rule {
        inputs: ["texttemplate.input"]
        outputFileTags: [product.texttemplate.outputTag]
        outputArtifacts: [
            {
                fileTags: [product.texttemplate.outputTag],
                filePath: input.texttemplate.outputFileName || input.completeBaseName
            }
        ]
        prepare: {
            var cmd = new JavaScriptCommand();
            cmd.silent = true;
            cmd.sourceCode = function() {
                try {
                    var src = new TextFile(input.filePath, TextFile.ReadOnly);
                    var dst = new TextFile(output.filePath, TextFile.WriteOnly);
                    var rex = /\${(\$|\w+)}/g;
                    var match;
                    while (!src.atEof()) {
                        rex.lastIndex = 0;
                        var line = src.readLine();
                        var matches = [];
                        while (match = rex.exec(line))
                            matches.push(match);
                        for (var i = matches.length; --i >= 0;) {
                            match = matches[i];
                            var replacement;
                            if (match[1] === "$") {
                                replacement = "$";
                            } else {
                                replacement = input.texttemplate.dict[match[1]];
                                if (typeof replacement === "undefined") {
                                    throw new Error("Placeholder '" + match[1]
                                                    + "' is not defined in textemplate.dict for '"
                                                    + input.fileName + "'.");
                                }
                            }
                            line = line.substr(0, match.index)
                                    + replacement
                                    + line.substr(match.index + match[0].length);
                        }
                        dst.writeLine(line);
                    }
                } finally {
                    if (src)
                        src.close();
                    if (dst)
                        dst.close();
                }
            };
            return [cmd];
        }
    }
}
