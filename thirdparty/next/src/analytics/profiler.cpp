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

#include "analytics/profiler.h"

#ifdef PROFILING_ENABLED

typedef list<Profiler::CallPoint>             CallsList;
typedef unordered_map<thread::id, CallsList>  ThreadMap;
typedef unordered_map<const char *, uint32_t> StatsMap;

static ThreadMap m_threads;
static StatsMap  m_stats;

Profiler::Profiler(const char *name) {
    m_current.name = name;
    m_current.started = std::chrono::high_resolution_clock::now();
}

Profiler::~Profiler() {
    m_current.stoped = std::chrono::high_resolution_clock::now();
    m_threads[this_thread::get_id()].push_back(m_current);
}

uint32_t Profiler::stat(const char *name) {
    return m_stats[name];
}

void Profiler::statAdd(const char *name, uint32_t value) {
    m_stats[name] += value;
}

void Profiler::statReset(const char *name) {
    m_stats[name] = 0;
}

#endif
