#include "analytics/profiler.h"

#ifdef PROFILING_ENABLED

typedef list<Profiler::CallPoint>             CallsList;
typedef unordered_map<thread::id, CallsList>  ThreadMap;
typedef unordered_map<const char *, uint32_t> StatsMap;

static ThreadMap m_Threads;
static StatsMap  m_Stats;

Profiler::Profiler(const char *name) {
    m_Current.name = name;
    m_Current.started = std::chrono::high_resolution_clock::now();
}

Profiler::~Profiler() {
    m_Current.stoped = std::chrono::high_resolution_clock::now();
    m_Threads[this_thread::get_id()].push_back(m_Current);
}

uint32_t Profiler::stat(const char *name) {
    return m_Stats[name];
}

void Profiler::statAdd(const char *name, uint32_t value) {
    m_Stats[name] += value;
}

void Profiler::statReset(const char *name) {
    m_Stats[name] = 0;
}

#endif
