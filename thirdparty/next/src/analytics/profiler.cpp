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
