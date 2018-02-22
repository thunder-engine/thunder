import ts = require("typescript");

declare var process: any;

export namespace io.qt.qbs {
    export class Artifact {
        filePath: string;
        fileTags: string[];
    }

    export namespace tools {
        export namespace utils {
            function stringEndsWith(s: string, e: string) {
                return s.slice(-e.length) === e;
            }

            export function artifactFromFilePath(filePath: string): Artifact {
                var fileTags: string[] = [];
                if (stringEndsWith(filePath, ".js.map")) {
                    fileTags.push("source_map");
                } else if (stringEndsWith(filePath, ".js")) {
                    fileTags.push("js", "compiled_typescript");
                } else if (stringEndsWith(filePath, ".d.ts")) {
                    fileTags.push("typescript_declaration");
                }

                return { filePath: filePath, fileTags: fileTags };
            }
        }

        function compileInternal(fileNames: string[], options: ts.CompilerOptions): qbs.Artifact[] {
            var outputArtifacts: qbs.Artifact[] = [];
            var program = ts.createProgram(fileNames, options);
            var emitResult = program.emit(undefined, filePath => {
                outputArtifacts.push(utils.artifactFromFilePath(filePath));
            });

            var allDiagnostics = ts.getPreEmitDiagnostics(program).concat(emitResult.diagnostics);
            allDiagnostics.forEach(diagnostic => {
                var message = ts.flattenDiagnosticMessageText(diagnostic.messageText, '\n');
                if (diagnostic.file) {
                    var { line, character } = diagnostic.file.getLineAndCharacterOfPosition(diagnostic.start);
                    console.error(`${diagnostic.file.fileName} (${line + 1},${character + 1}): ${message}`);
                } else {
                    console.error(message);
                }
            });

            return emitResult.emitSkipped ? undefined : outputArtifacts;
        }

        export function compile(commandLineArguments: string[]): qbs.Artifact[] {
            var parsedCommandLine = ts.parseCommandLine(commandLineArguments);
            return compileInternal(parsedCommandLine.fileNames, parsedCommandLine.options);
        }

        export function TypeScriptCompilerScannerToolMain(): void {
            var outputArtifacts = compile(process.argv.slice(2));
            if (outputArtifacts !== undefined) {
                console.log(JSON.stringify(outputArtifacts));
            } else {
                process.exit(1);
            }
        }
    }
}

io.qt.qbs.tools.TypeScriptCompilerScannerToolMain();
