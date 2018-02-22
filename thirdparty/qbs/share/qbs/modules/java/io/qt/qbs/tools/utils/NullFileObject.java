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

package io.qt.qbs.tools.utils;

import javax.lang.model.element.Modifier;
import javax.lang.model.element.NestingKind;
import javax.tools.FileObject;
import javax.tools.JavaFileObject;
import java.io.*;
import java.net.URI;

/**
 * Represents a FileObject that discards its output when written.
 */
public class NullFileObject implements FileObject, JavaFileObject {
    FileObject obj;

    public NullFileObject(FileObject obj) {
        this.obj = obj;
    }

    @Override
    public URI toUri() {
        return obj.toUri();
    }

    @Override
    public String getName() {
        return obj.getName();
    }

    @Override
    public InputStream openInputStream() throws IOException {
        return obj.openInputStream();
    }

    @Override
    public OutputStream openOutputStream() throws IOException {
        return new OutputStream() {
            @Override
            public void write(int b) throws IOException {
            }
        };
    }

    @Override
    public Reader openReader(boolean ignoreEncodingErrors) throws IOException {
        return obj.openReader(ignoreEncodingErrors);
    }

    @Override
    public CharSequence getCharContent(boolean ignoreEncodingErrors)
            throws IOException {
        return obj.getCharContent(ignoreEncodingErrors);
    }

    @Override
    public Writer openWriter() throws IOException {
        return new Writer() {
            @Override
            public void write(char[] cbuf, int off, int len) throws IOException {
            }

            @Override
            public void flush() throws IOException {
            }

            @Override
            public void close() throws IOException {
            }
        };
    }

    @Override
    public long getLastModified() {
        return obj.getLastModified();
    }

    @Override
    public boolean delete() {
        return true;
    }

    @Override
    public Kind getKind() {
        if (obj instanceof JavaFileObject) {
            return ((JavaFileObject) obj).getKind();
        }

        throw new UnsupportedOperationException();
    }

    @Override
    public boolean isNameCompatible(String simpleName, Kind kind) {
        if (obj instanceof JavaFileObject) {
            return ((JavaFileObject) obj).isNameCompatible(simpleName, kind);
        }

        throw new UnsupportedOperationException();
    }

    @Override
    public NestingKind getNestingKind() {
        if (obj instanceof JavaFileObject) {
            return ((JavaFileObject) obj).getNestingKind();
        }

        throw new UnsupportedOperationException();
    }

    @Override
    public Modifier getAccessLevel() {
        if (obj instanceof JavaFileObject) {
            return ((JavaFileObject) obj).getAccessLevel();
        }

        throw new UnsupportedOperationException();
    }
}
