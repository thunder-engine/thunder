/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2014 Evgeny Prikazchikov
*/

#ifndef FILEIO_H
#define FILEIO_H

#include <stdint.h>
#include <string>
#include <list>

#include <global.h>

using namespace std;

typedef void            _FILE;
typedef	uint64_t        _size_t;
typedef list<string>    StringList;

class NEXT_LIBRARY_EXPORT File {
public:
    void                finit           (const char *argv0);
    void                fsearchPathAdd  (const char *path, bool isFirst = false);

    virtual StringList  flist          (const char *path);

    virtual bool        mkdir          (const char *path);

    virtual bool        fdelete        (const char *path);

    virtual bool        exists         (const char *path);

    virtual bool        isdir          (const char *path);

    virtual int         fclose         (_FILE *stream);

    virtual _size_t     fseek          (_FILE *stream, uint64_t origin);

    virtual _FILE      *fopen          (const char *path, const char *mode);

    virtual _size_t     fread          (void *ptr, _size_t size, _size_t count, _FILE *stream);

    virtual _size_t     fwrite         (const void *ptr, _size_t size, _size_t count, _FILE *stream);

    virtual _size_t     fsize          (_FILE *stream);

    virtual _size_t     ftell          (_FILE *stream);
};

#endif // FILEIO_H
