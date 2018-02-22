/****************************************************************************
 **
 ** Copyright (C) 2015 Jake Petroules.
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

package io.qt.qbs;

import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.List;

/**
 * This uses a custom JSON implementation because the Java Standard Library does
 * not yet have native support for JSON, and only minimal support is required
 * here.
 */
public class ArtifactListJsonWriter implements ArtifactListWriter {
    private static final int TAB_WIDTH = 4;

    // based on escapeString from qtbase/qjsonwriter.cpp
    private static String escapeString(String s) {
        String out = "";
        for (int i = 0; i < s.length();) {
            int u = s.codePointAt(i);

            // unpaired surrogate
            if (u >= Character.MIN_SURROGATE && u <= Character.MAX_SURROGATE) {
                out += "\ufffd";
                i += Character.charCount(u);
                continue;
            }

            if (u < 0x80) {
                if (u < 0x20 || u == 0x22 || u == 0x5c) {
                    out += "\\";
                    switch (u) {
                    case 0x22:
                        out += "\"";
                        break;
                    case 0x5c:
                        out += "\\";
                        break;
                    case 0x8:
                        out += "b";
                        break;
                    case 0xc:
                        out += "f";
                        break;
                    case 0xa:
                        out += "n";
                        break;
                    case 0xd:
                        out += "r";
                        break;
                    case 0x9:
                        out += "t";
                        break;
                    default:
                        out += "u";
                        out += "0";
                        out += "0";
                        String hex = Integer.toHexString(u);
                        if (hex.length() == 1)
                            out += "0";
                        out += hex;
                        break;
                    }
                } else {
                    out += s.substring(i, i + Character.charCount(u));
                }
            } else {
                out += s.substring(i, i + Character.charCount(u));
            }

            i += Character.charCount(u);
        }

        return out;
    }

    private static void writeString(PrintStream printWriter, String s) {
        printWriter.print("\"");
        printWriter.print(escapeString(s));
        printWriter.print("\"");
    }

    private static void writeIndent(PrintStream printWriter, int level) {
        for (int i = 0; i < level * TAB_WIDTH; ++i) {
            printWriter.print(" ");
        }
    }

    private static void writeArtifact(Artifact artifact,
            PrintStream printWriter, int indentLevel, Boolean comma) {
        writeIndent(printWriter, indentLevel++);
        printWriter.print("{\n");
        writeIndent(printWriter, indentLevel);
        writeString(printWriter, "filePath");
        printWriter.print(": ");
        writeString(printWriter, artifact.getFilePath());
        printWriter.println(",");
        writeIndent(printWriter, indentLevel);
        writeString(printWriter, "fileTags");
        printWriter.print(": [");
        for (int i = 0; i < artifact.getFileTags().size(); ++i) {
            writeString(printWriter, artifact.getFileTags().get(i));
            if (i != artifact.getFileTags().size() - 1)
                printWriter.print(", ");
        }
        printWriter.println("]");
        writeIndent(printWriter, --indentLevel);
        printWriter.println("}" + (comma ? "," : ""));
    }

    @Override
    public void write(List<Artifact> artifacts, OutputStream outputStream)
            throws IOException {
        PrintStream printWriter = new PrintStream(outputStream);
        printWriter.print("[\n");
        for (int i = 0; i < artifacts.size(); ++i) {
            writeArtifact(artifacts.get(i), printWriter, 1,
                    i != artifacts.size() - 1);
        }

        printWriter.println("]");
    }
}
