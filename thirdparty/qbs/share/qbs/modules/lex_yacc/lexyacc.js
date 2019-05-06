var FileInfo = require("qbs.FileInfo");
var TextFile = require("qbs.TextFile");

function unquote(s)
{
    return s.startsWith('"') && s.endsWith('"') ? s.substr(1, s.length - 2) : s;
}

function readLexOptions(filePath) {
    var result = {};
    var f = new TextFile(filePath, TextFile.ReadOnly);
    var regex = /^%option\s+([^ \t=]+)(?:\s*=\s*(\S+))?/;
    while (!f.atEof()) {
        var line = f.readLine();
        var m = regex.exec(line);
        if (!m) {
            if (line === "%%")
                break;
            continue;
        }
        result[m[1]] = m[2] || true;
    }
    f.close();
    return result;
}

function lexOutputFilePath(input, posixFileName, options)
{
    var outDir = input.lex_yacc.outputDir;
    var fileName;
    if (options.outfile) {
        fileName = unquote(options.outfile);
    } else if (options.prefix) {
        fileName = FileInfo.baseName(posixFileName) + '.'
                + unquote(options.prefix) + '.'
                + FileInfo.suffix(posixFileName);
    } else if (input.lex_yacc.uniqueSymbolPrefix) {
        fileName = input.baseName;
        fileName += posixFileName;
    } else {
        fileName = posixFileName;
    }
    return FileInfo.joinPaths(outDir, fileName);
}

function readYaccOptions(filePath) {
    var result = {};
    var f = new TextFile(filePath, TextFile.ReadOnly);
    var regex = /^%output\s+(.+)/;
    while (!f.atEof()) {
        var line = f.readLine();
        var m = regex.exec(line);
        if (!m) {
            if (line === "%%")
                break;
            continue;
        }
        result.output = m[1];
        break;
    }
    f.close();
    return result;
}

function yaccOutputFilePath(input, posixFileName, options)
{
    var outDir = input.lex_yacc.outputDir;
    var fileName;
    if (options.output) {
        var outputFileName = unquote(options.output);
        var suffix = FileInfo.suffix(posixFileName);
        if (suffix === "c") {
            fileName = outputFileName;
        } else {
            fileName = FileInfo.completeBaseName(outputFileName)
                    + '.' + suffix + FileInfo.suffix(outputFileName).slice(1);
        }
    } else if (input.lex_yacc.uniqueSymbolPrefix) {
        fileName = input.baseName + posixFileName.slice(1);
    } else {
        fileName = posixFileName;
    }
    return FileInfo.joinPaths(outDir, fileName);
}
