/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2023 Evgeniy Prikazchikov
*/

#ifndef PROFILER
#define PROFILER

#ifdef PROFILING_ENABLED

#include <thread>
#include <unordered_map>

#include "engine.h"
#include "timer.h"

class NEXT_LIBRARY_EXPORT Profiler {
public:
    struct CallPoint {
        const char *name;

        TimePoint started;

        TimePoint stoped;
    };

public:
    Profiler(const char *name);

    ~Profiler();

    static uint32_t stat(const char *name);

    static void statAdd(const char *name, uint32_t value);

    static void statReset(const char *name);

protected:
    Profiler::CallPoint  m_current;

};

#endif

#endif // PROFILER

